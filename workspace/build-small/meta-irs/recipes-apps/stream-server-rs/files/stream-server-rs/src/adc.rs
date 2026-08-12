use std::collections::BTreeMap;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use crate::error::BackendError;
use crate::types::{AdcConfig, ChannelConfigState, ConfigValue};

use super::Result;

use industrial_io as iio;
use serde_json::{json, Value};
use tracing::{debug, error, info, trace};

pub struct Adc {
    ctx: iio::Context,
    uri: String,
    dev_name: String,
}

impl Adc {
    pub fn open(uri: &str, device_name: &str) -> Result<Self> {
        let ctx = iio::Context::from_uri(uri)?;

        let me = Self {
            ctx,
            uri: uri.to_string(),
            dev_name: device_name.to_string(),
        };

        let dev = me.dev()?;
        for ch in dev.channels() {
            if ch.is_scan_element() {
                ch.enable();
            }
        }

        Ok(me)
    }

    pub fn uri(&self) -> &str {
        &self.uri
    }

    pub fn device_name(&self) -> &str {
        &self.dev_name
    }

    fn dev(&self) -> Result<iio::Device> {
        debug!("Searching device: {}", self.dev_name);
        self.ctx.find_device(&self.dev_name)
            .ok_or(BackendError::InvalidIioDevice(self.dev_name.to_string()))
    }

    pub fn set_config(&self, channel_name: &String, parameter: &str, value: &str)-> Result<()> {
        debug!("Setting config for channel: {}", channel_name);

        let dev = self.dev()?;
        let chan = dev.channels()
            .find(|c| c.id().as_ref() == Some(channel_name))
            .ok_or(BackendError::InvalidChannelId(channel_name.clone()))?;

        let mut parameter_available = parameter.to_string();
        parameter_available.push_str("_available");

        let available = chan.attr_read_str(&parameter_available)?;
        let available_parsed: std::result::Result<ConfigValue, _> = available.parse();

        match available_parsed {
            Ok(ConfigValue::List(_parsed)) => {

                // TODO: `parsed is unused because I dont know how to check the value is a list or a range.
                // Not sure is the iio backend tells me anywhere

                // if parsed.contains(&value) {
                    let r = chan.attr_write_str(parameter, value);
                    debug!("Setting channel result: {:?} ({}, {})", r, parameter, value);
                    Ok(())
                // } else {
                    // Err(
                        // BackendError::InvalidConfigValue(
                            // ConfigValue::Number(value),
                            // ConfigValue::List(parsed)
                        // )
                    // )
                // }
            },
            err => {
                error!("Could not read available parameters: {:?}", err);
                Err(BackendError::InvalidAvailableValues(channel_name.to_string(), parameter.to_string()))
            }
        }
    }

    pub fn config(&self) -> Result<AdcConfig> {
        let dev = self.dev()?;
        let chans: Vec<_> = dev.channels().filter(|c| c.is_enabled()).collect();

        let read_avail = |c: &iio::Channel, attr: &str| -> Option<Vec<String>> {
            c.attr_read_str(attr)
                .map(|s| parse_available(&s)).ok()
        };

        let mut out: AdcConfig = BTreeMap::new();
        for (idx, ch) in chans.iter().enumerate() {
            let name = ch
                .id()
                .or_else(|| ch.name())
                .unwrap_or_else(|| format!("channel{idx}"));
            out.insert(
                name,
                ChannelConfigState {
                    scale: ch.attr_read_str("scale").ok(),
                    scale_available: read_avail(ch, "scale_available"),
                    calibbias: ch.attr_read_str("calibbias").ok(),
                    calibbias_available: read_avail(ch, "calibbias_available"),
                    convdelay: ch.attr_read_str("convdelay").ok(),
                    convdelay_available: read_avail(ch, "convdelay_available"),
                },
            );
        }
        Ok(out)
    }

    /// Continuously reads chunks of `chunk_size` samples from a freshly opened
    /// context and forwards each chunk as JSON to `tx`. Stops when `stop`
    /// becomes true or the receiver is dropped.
    pub fn run_stream(
        uri: String,
        device_name: String,
        chunk_size: usize,
        sample_rate: i64,
        tx: tokio::sync::mpsc::Sender<Value>,
        stop: Arc<AtomicBool>,
        pre_scale: Option<f64>,
        raw_value: bool
    ) -> Result<()> {

        let ctx = iio::Context::from_uri(&uri)?;
        let dev = ctx
            .find_device(&device_name)
            .ok_or(BackendError::InvalidIioDevice(device_name))?;

        for ch in dev.channels() {
            if ch.is_scan_element() {
                info!("Enabling channel {:?}", ch.id());
                ch.enable();
            }
        }
        if sample_rate > 0 {
            trace!("Setting sampling_frequency to {}", sample_rate);
            let _ = dev.attr_write_int("sampling_frequency", sample_rate);
        }

        trace!("Creating iio buffer with size: {}", chunk_size);
        let mut buf = dev.create_buffer(chunk_size, false)?;

        trace!("Starting read loop");
        while !stop.load(Ordering::Relaxed) {
            buf.refill()?;

            let mut out = serde_json::Map::new();
            for ch in dev.channels().filter(|c| c.is_enabled()) {
                let raw: Vec<i32> = ch.read(&buf)?;

                let scale = if raw_value {
                    1.0
                } else {
                    ch.attr_read_float("scale").unwrap_or(1.0)
                };

                let scaled: Vec<f64> =
                    raw.iter().map(|v| (*v as f64) * scale * pre_scale.unwrap_or(1f64)).collect();
                out.insert(ch.id().unwrap_or_default(), json!(scaled));
            }

            if tx.blocking_send(Value::Object(out)).is_err() {
                trace!("Sending sampled value to ws_loop");
                break;
            }
        }
        Ok(())
    }
}

fn parse_available(raw: &str) -> Vec<String> {
    raw.split(|c: char| {
        !c.is_ascii_digit() && c != '.' && c != '-' && c != 'e' && c != 'E' && c != '+'
    })
    .filter(|s| !s.is_empty())
    .map(|s| s.to_string())
    .collect()
}
