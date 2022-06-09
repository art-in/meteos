use std::{error::Error, fs::File, io::Read};

#[derive(serde::Deserialize)]
pub struct Config {
    pub meteos_backend_url: String,
    pub tg_bot_token: String,
    pub check_interval_sec: u64,
    pub latest_samples_period_sec: u64,
    pub backend_error_timeout_sec: u64,
}

impl Config {
    pub fn read() -> Result<Self, Box<dyn Error>> {
        let mut str = String::new();
        File::open("./config.toml")?.read_to_string(&mut str)?;
        Ok(toml::from_str(&str)?)
    }
}
