use axum::{http::StatusCode, response::{IntoResponse, Response}};
use tracing::error;

use crate::types::{ChannelId, ConfigValue, DeviceName, Parameter};


#[derive(Debug)]
pub enum BackendError {
    InvalidIioDevice(DeviceName),
    InvalidConfigValue(ConfigValue, ConfigValue), // wanted, available
    InvalidChannelId(ChannelId),
    InvalidAvailableValues(ChannelId, Parameter),
    IIOReadConfigFailed,
    IIOSetConfigFailed,
    IIOError(String),
    RuntimeError,

    InvalidChannel,
    Generic(String)
}

impl IntoResponse for BackendError {
    fn into_response(self) -> Response {
        (StatusCode::SEE_OTHER, format!("{:?}", self)).into_response()
    }
}

impl From<industrial_io::Error> for BackendError {
    fn from(value: industrial_io::Error) -> Self {
        error!("An error occured in the IndustrialIO Module: {:?}", value);
        Self::IIOError(value.to_string())
    }
}

impl From<std::io::Error> for BackendError {
    fn from(value: std::io::Error) -> Self {
        error!("Error: {:?}", value);
        Self::RuntimeError
    }
}
