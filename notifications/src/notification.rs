use crate::backend_api::{self, Reading, Sample};
use std::{fmt::Debug, time::Duration};

#[derive(Debug)]
pub struct EnvOutOfNormNotification {
    pub readings_out_or_range: Vec<Reading>,
    pub last_sample: Sample,
}

#[derive(Debug)]
pub struct BackendErrorNotification {
    pub last_error: backend_api::Error,
    pub error_period: Duration,
    pub error_count: u32,
}

pub trait Notification: Debug {
    fn get_message(&self) -> String;
}

impl Notification for EnvOutOfNormNotification {
    fn get_message(&self) -> String {
        let readings: Vec<String> = self
            .readings_out_or_range
            .iter()
            .map(|r| format!("{}", r))
            .collect();
        let readings = readings.join(", ");

        format!(
            "{readings} are out of normal range\n\
            \n\
            {last_sample}",
            last_sample = self.last_sample
        )
    }
}

impl Notification for BackendErrorNotification {
    fn get_message(&self) -> String {
        format!(
            "Error: \
            {count} backend requests have failed in last {period} minutes. \
            Last error: {last_error}",
            count = self.error_count,
            period = (self.error_period.as_secs() / 60) as u32,
            last_error = self.last_error
        )
    }
}
