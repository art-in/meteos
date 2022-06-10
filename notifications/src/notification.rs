use crate::backend_api::{self, Reading, Sample};
use std::{fmt::Debug, time::Duration};

#[derive(Debug)]
pub struct EnvOutOfRangeNotification {
    pub readings_out_or_range: Vec<Reading>,
    pub latest_sample: Sample,
}

#[derive(Debug)]
pub struct BackendErrorNotification {
    pub latest_error: backend_api::BackendApiError,
    pub error_period: Duration,
    pub error_count: u32,
}

pub struct NotificationMessage {
    pub format: NotificationMessageFormat,
    pub text: String,
}

pub enum NotificationMessageFormat {
    Html,
    Markdown,
}

pub trait Notification: Debug {
    fn get_message(&self) -> NotificationMessage;
}

impl Notification for EnvOutOfRangeNotification {
    fn get_message(&self) -> NotificationMessage {
        let readings: Vec<String> = self
            .readings_out_or_range
            .iter()
            .map(|r| format!("{}", r))
            .collect();
        let readings = readings.join(", ");

        NotificationMessage {
            format: NotificationMessageFormat::Markdown,
            text: format!(
                "{readings} is \\(are\\) out of normal range\n\n\
                {latest_sample}",
                latest_sample = self.latest_sample
            ),
        }
    }
}

impl Notification for BackendErrorNotification {
    fn get_message(&self) -> NotificationMessage {
        NotificationMessage {
            format: NotificationMessageFormat::Html,
            text: format!(
                "ERROR: \
                {count} backend request(s) have failed in the last {period} minute(s). \
                Latest error: {latest_error}",
                count = self.error_count,
                period = (self.error_period.as_secs() / 60) as u32,
                latest_error = self.latest_error
            ),
        }
    }
}
