use crate::config::Config;
use serde::Deserialize;
use std::{
    fmt::{Display, Formatter},
    sync::Arc,
    time::Duration,
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

#[derive(Debug)]
pub enum Error {
    UnreachableApi(reqwest::Error),
    ResponseDeserializationFailed(reqwest::Error),
    NoEnvSamples,
}

impl Display for Error {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        let message = match self {
            Error::UnreachableApi(_) => "Failed to reach Meteos API",
            Error::ResponseDeserializationFailed(_) => "Failed to parse Meteos API response",
            Error::NoEnvSamples => "No environment samples received from Meteos API",
        };
        write!(f, "{}", message)
    }
}

pub struct BackendApi {
    config: Arc<Config>,
}

impl BackendApi {
    pub fn new(config: Arc<Config>) -> Self {
        BackendApi { config }
    }

    pub async fn get_latest_samples(&self) -> Result<Vec<Sample>, Error> {
        let backend_url = &self.config.meteos_backend_url;

        let now: chrono::DateTime<chrono::Utc> = std::time::SystemTime::now().into();
        let period = chrono::Duration::seconds(self.config.latest_samples_period_sec as i64);
        let from = now
            .checked_sub_signed(period)
            .unwrap()
            .to_rfc3339_opts(chrono::SecondsFormat::Secs, true);

        let request_url = format!("{backend_url}/samples?from={from}");
        let response = reqwest::get(request_url)
            .await
            .map_err(Error::UnreachableApi)?;

        let samples = response
            .json::<Vec<Sample>>()
            .await
            .map_err(Error::ResponseDeserializationFailed)?;

        if samples.is_empty() {
            Err(Error::NoEnvSamples)
        } else {
            Ok(samples)
        }
    }
}
