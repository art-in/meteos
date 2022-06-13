use anyhow::Result;
use std::{fs::File, io::Read, ops::Range};

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
    pub check_interval_sec: u64,
    pub check_period_sec: u64,
    pub backend_error_timeout_sec: u64,
    pub optimal_ranges: EnvironmentalReadingRanges,
}

impl Config {
    pub fn read() -> Result<Self> {
        let mut str = String::new();
        File::open("./config.toml")?.read_to_string(&mut str)?;
        Ok(toml::from_str(&str)?)
    }
}
