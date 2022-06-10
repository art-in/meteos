use crate::config::Config;
use chrono::{DateTime, Utc};
use reqwest::{Response, StatusCode};
use serde::Deserialize;
use std::{
    fmt::{Display, Formatter},
    sync::Arc,
};

#[derive(Deserialize, Debug, Clone)]
pub struct Sample {
    #[serde(alias = "c")]
    pub co2: f64,
    #[serde(alias = "h")]
    pub humidity: f64,
    #[serde(alias = "p")]
    pub pressure: f64,
    #[serde(alias = "t")]
    pub temperature: f64,
    #[serde(alias = "u")]
    pub time: String,
}

impl Display for Sample {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "```\n\
            🌡 temperature: {} °C\n\
            💧 humidity:    {} %\n\
            🔨 pressure:    {} mm/hg\n\
            💨 co2:         {} ppm\n\
            ```",
            self.temperature, self.humidity, self.pressure, self.co2
        )
    }
}

#[derive(Debug)]
pub enum Reading {
    Co2,
    Humidity,
    Pressure,
    Temperature,
}

impl Display for Reading {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        let message = match self {
            Reading::Co2 => "co2",
            Reading::Humidity => "humidity",
            Reading::Pressure => "pressure",
            Reading::Temperature => "temperature",
        };
        write!(f, "{}", message)
    }
}

#[derive(thiserror::Error, Debug)]
pub enum BackendApiError {
    #[error("failed to reach Meteos API: {}", .source)]
    UnreachableApi { source: UnreachableApiError },
    #[error("failed to parse Meteos API response: {}", .source)]
    ResponseDeserializationFailed { source: reqwest::Error },
    #[error("no environment samples received from Meteos API")]
    NoEnvSamples,
}

#[derive(thiserror::Error, Debug)]
pub enum UnreachableApiError {
    #[error("invalid status code \"{}\"", .0)]
    InvalidStatusCode(StatusCode),
    #[error(transparent)]
    Other {
        #[from]
        source: reqwest::Error,
    },
}

pub struct BackendApi {
    config: Arc<Config>,
}

impl BackendApi {
    pub fn new(config: Arc<Config>) -> Self {
        BackendApi { config }
    }

    async fn request(&self, path: &str) -> Result<Response, BackendApiError> {
        let backend_url = &self.config.meteos_backend_url;
        let request_url = format!("{backend_url}/{path}");
        let response =
            reqwest::get(request_url)
                .await
                .map_err(|err| BackendApiError::UnreachableApi {
                    source: UnreachableApiError::Other { source: err },
                })?;

        if response.status() != StatusCode::OK {
            return Err(BackendApiError::UnreachableApi {
                source: UnreachableApiError::InvalidStatusCode(response.status()),
            });
        }

        Ok(response)
    }

    pub async fn get_samples(&self, from: DateTime<Utc>) -> Result<Vec<Sample>, BackendApiError> {
        let from = from.to_rfc3339_opts(chrono::SecondsFormat::Secs, true);
        let response = self.request(&format!("samples?from={from}")).await?;
        let samples = response
            .json::<Vec<Sample>>()
            .await
            .map_err(|err| BackendApiError::ResponseDeserializationFailed { source: err })?;

        if samples.is_empty() {
            Err(BackendApiError::NoEnvSamples)
        } else {
            Ok(samples)
        }
    }

    pub async fn get_latest_samples(&self) -> Result<Vec<Sample>, BackendApiError> {
        let now: chrono::DateTime<chrono::Utc> = std::time::SystemTime::now().into();
        let period = chrono::Duration::seconds(self.config.latest_samples_period_sec as i64);
        let from = now
            .checked_sub_signed(period)
            .expect("failed to substruct duration");

        self.get_samples(from).await
    }
}
