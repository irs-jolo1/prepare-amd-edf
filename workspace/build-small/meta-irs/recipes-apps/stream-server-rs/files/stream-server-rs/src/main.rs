use crate::error::BackendError;

mod adc;
mod server;
mod error;
mod types;
mod macros;

pub type Result<T> = std::result::Result<T, BackendError>;

#[tokio::main]
async fn main() -> Result<()> {
    tracing_subscriber::fmt()
        .with_env_filter(
            tracing_subscriber::EnvFilter::try_from_default_env()
                .unwrap_or_else(|_| "info".into()),
        )
        .init();

    server::run("0.0.0.0:8082").await
}
