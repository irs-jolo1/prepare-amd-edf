use std::{collections::BTreeMap, str};
use serde::{Deserialize, Serialize};

use axum::{Json, http::StatusCode, response::{IntoResponse, Response}};
use serde_json::json;
use tracing::{error, trace};

pub type ChannelId = String;
pub type DeviceName = String;
pub type Parameter = String;
pub type ConfigSetValues = BTreeMap<String, BTreeMap<String, String>>;

pub type ConfigValueFromStr = ();

pub type AdcConfig = BTreeMap<String, ChannelConfigState>;

#[derive(Serialize, Deserialize, Debug)]
pub struct ChannelConfigState {
    pub scale: Option<String>,
    pub scale_available: Option<Vec<String>>,
    pub calibbias: Option<String>,
    pub calibbias_available: Option<Vec<String>>,
    pub convdelay: Option<String>,
    pub convdelay_available: Option<Vec<String>>,
}

#[derive(Debug, Clone)]
pub enum ConfigValue {
    Number(f64),
    Range(f64, f64, f64),
    List(Vec<f64>),
}

fn parse_available_string(raw: &str) -> Vec<Option<f64>> {
    raw.split(|c: char| {
        !c.is_ascii_digit() && c != '.' && c != '-' && c != 'e' && c != 'E' && c != '+'
    })
    .filter(|s| !s.is_empty())
    .map(|s| s.parse::<f64>().ok())
    .collect()
}


impl str::FromStr for ConfigValue {
    type Err = ConfigValueFromStr;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        trace!("Parsing to `{}` to ConfigValue", s);

        let float = s.parse::<f64>();
        if let Ok(n) = float {
            trace!("String can be parsed as f64: {}", n);
            return Ok(Self::Number(n))
        }

        let list = parse_available_string(s);

        let list_mapped: Option<Vec<f64>> = list.into_iter().collect();
        trace!("list_mapped: {:?}", list_mapped);
        if let Some(n) = list_mapped {
            trace!("String can be parsed as f64 list: {:?}", n);
            return Ok(Self::List(n))
        }

        error!("Could not parsed &str");
        Err(())
    }
}

pub enum BackendResponse {
    Config(AdcConfig),
}

impl IntoResponse for BackendResponse {
    fn into_response(self) -> Response {
        match self {
            Self::Config(cfg) => (
                StatusCode::OK,
                Json(json!({ "status": "ok", "config": cfg }))
            ).into_response()
        }
    }
}
