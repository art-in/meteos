use serde::Deserialize;
use std::{
    fmt::{Display, Formatter},
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

pub async fn get_latest_samples(from_ago: Duration) -> Result<Vec<Sample>, Error> {
    let now: chrono::DateTime<chrono::Utc> = std::time::SystemTime::now().into();
    let from_ago = chrono::Duration::from_std(from_ago).unwrap();
    let from = now
        .checked_sub_signed(from_ago)
        .unwrap()
        .to_rfc3339_opts(chrono::SecondsFormat::Secs, true);

    let backend_url = std::env::var("METEOS_BACKEND_URL").expect("METEOS_BACKEND_URL");
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
