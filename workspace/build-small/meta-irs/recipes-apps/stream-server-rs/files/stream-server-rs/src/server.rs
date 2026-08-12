use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;

use axum::extract;
use axum::{
    extract::{
        ws::{Message, WebSocket, WebSocketUpgrade},
        Query, State,
    },
    http::{Method},
    response::IntoResponse,
    routing::get, Router,
};
use serde::Deserialize;
use serde_json::json;
use tokio::sync::Mutex;
use tower_http::cors::{Any, CorsLayer};
use tracing::{error, info, trace};
use crate::{Result, blocking};

use crate::adc::{Adc};
use crate::error::BackendError;
use crate::types::{AdcConfig, ConfigSetValues, BackendResponse};

type AppState = Arc<Mutex<Option<Adc>>>;

const STREAM_CHUNK_SAMPLES: usize = 1000;

#[derive(Deserialize, Debug)]
struct ConfigQuery {
    uri: String,
    device_name: String,
}

#[derive(Deserialize)]
struct WsQuery {
    #[serde(default)]
    sample_rate: i64,
    #[serde(default)]
    buffer_size: Option<usize>,
    #[serde(default)]
    pre_scale: Option<f64>,
    #[serde(default)]
    raw_values: bool,
}

pub async fn run(addr: &str) -> Result<()> {
    let state: AppState = Arc::new(Mutex::new(None));

    let cors = CorsLayer::new()
        .allow_origin(Any)
        .allow_methods([Method::GET, Method::POST, Method::OPTIONS])
        .allow_headers(Any);

    let app = Router::new()
        .route("/config", get(get_config).post(post_config))
        .route("/ws", get(ws_handler))
        .layer(cors)
        .with_state(state);

    let listener = tokio::net::TcpListener::bind(addr).await?;
    info!("Listening on {addr}");
    axum::serve(listener, app).await?;
    Ok(())
}

// config endpoint
async fn get_config(
    State(state): State<AppState>,
    Query(q): Query<ConfigQuery>,
) -> Result<BackendResponse> {

    let res = tokio::task::spawn_blocking(move || {
        let adc = Adc::open(&q.uri, &q.device_name)?;
        let cfg = adc.config()?;
        Ok::<(Adc, AdcConfig), BackendError>((adc, cfg))
    })
    .await;

    match res {
        Ok(Ok((adc, cfg))) => {
            *state.lock().await = Some(adc);
            Ok(BackendResponse::Config(cfg))
        }
        Ok(Err(e)) => {
            error!("Could not read iio config: {:?}", e);
            Err(BackendError::IIOReadConfigFailed)
        }
        Err(e) => {
            error!("Could not spawn ADC blocking thread: {:?}", e);
            Err(BackendError::RuntimeError)
        }
    }
}

async fn post_config(
    // State(state): State<AppState>,
    Query(q): Query<ConfigQuery>,
    extract::Json(body): extract::Json<ConfigSetValues>,
) -> Result<BackendResponse> {

    blocking!(
        "Could not set ADC config",
        move || -> Result<AdcConfig> {
            let adc = Adc::open(&q.uri, &q.device_name)?;
            for (channel_id, channel_config) in body.iter() {
                for (parameter, value) in channel_config.iter() {
                    info!("Setting channel ({}) parameter ({}) to {}", channel_id, parameter, value);
                    adc.set_config(channel_id, parameter, value)?;
                }
            }
            adc.config()
        }
    )
}

async fn ws_handler(
    ws: WebSocketUpgrade,
    Query(q): Query<WsQuery>,
    State(state): State<AppState>,
) -> impl IntoResponse {
    ws.on_upgrade(move |socket| ws_loop(socket, state, q))
}

async fn ws_loop(mut socket: WebSocket, state: AppState, q: WsQuery) {
    let chunk = q.buffer_size.unwrap_or(STREAM_CHUNK_SAMPLES);
    info!(
        "WS connected (sample_rate={}, chunk_samples={}, pre_scale={:?}, raw_values={})",
        q.sample_rate,
        chunk,
        q.pre_scale,
        q.raw_values,
    );

    let (uri, dev_name) = {
        let g = state.lock().await;
        trace!("State lock aqquired");
        match g.as_ref() {
            Some(a) => (a.uri().to_string(), a.device_name().to_string()),
            None => {
                trace!("Lock was None!");
                let _ = socket
                    .send(Message::Text(
                        json!({"status":"error","detail":"not initialized"})
                            .to_string(),
                    ))
                    .await;
                return;
            }
        }
    };
    trace!("uri and device name");

    let stop = Arc::new(AtomicBool::new(false));
    let (tx, mut rx) = tokio::sync::mpsc::channel::<serde_json::Value>(100);

    let stop_thread = stop.clone();
    let sr = q.sample_rate;
    let pre_scale = q.pre_scale;
    let raw_values = q.raw_values;
    trace!("Starting ADC worker thread");
    let worker = std::thread::spawn(move || {
        if let Err(e) =
            Adc::run_stream(uri, dev_name, chunk, sr, tx, stop_thread, pre_scale, raw_values)
        {
            error!("stream thread error: {:?}", e);
        }
    });

    loop {
        tokio::select! {
            maybe = rx.recv() => {
                match maybe {
                    Some(v) => {
                        let payload = json!({"status":"ok","voltages": v});
                        if socket
                            .send(Message::Text(payload.to_string()))
                            .await
                            .is_err()
                        {
                            break;
                        }
                    }
                    None => break, // worker exited
                }
            }
            msg = socket.recv() => {
                match msg {
                    Some(Ok(Message::Close(_))) | None => break,
                    Some(Err(_)) => break,
                    _ => {}
                }
            }
        }
    }

    stop.store(true, Ordering::Relaxed);
    drop(rx);
    let _ = worker.join();
    info!("WS disconnected");
}
