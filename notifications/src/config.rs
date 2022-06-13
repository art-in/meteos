use anyhow::Result;
use std::{fs::File, io::Read, ops::Range, time::Duration};

#[derive(serde::Deserialize)]
pub struct EnvironmentalReadingRanges {
    pub temperature: Range<f64>,
    pub humidity: Range<f64>,
    pub co2: Range<f64>,
    pub pressure: Range<f64>,
}

#[derive(serde::Deserialize)]
pub struct Config {
    pub backend_url: String,
    pub tg_bot_token: String,
    check_interval_sec: u64,
    check_period_sec: u64,
    backend_error_timeout_sec: u64,
    pub optimal_ranges: EnvironmentalReadingRanges,

    #[serde(skip)]
    pub check_interval: Duration,

    #[serde(skip)]
    pub check_period: Duration,

    #[serde(skip)]
    pub backend_error_timeout: Duration,
}

impl Config {
    pub fn read() -> Result<Self> {
        let mut str = String::new();
        File::open("./config.toml")?.read_to_string(&mut str)?;
        let mut config: Config = toml::from_str(&str)?;

        config.check_interval = Duration::from_secs(config.check_interval_sec);
        config.check_period = Duration::from_secs(config.check_period_sec);
        config.backend_error_timeout = Duration::from_secs(config.backend_error_timeout_sec);

        Ok(config)
    }
}
