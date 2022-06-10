use crate::{
    backend_api::{self, Reading, Sample},
    tg_bot::{GetTgMessage, TgMessage, TgMessageFormat},
};
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

// telegram message is the only representation supported for now.
// work on more notification channels in future may require to extend
// Notification to support more representations. eg.:
// trait Notification: GetTgMessage + GetEmailMessage + GetSmsMessage {}
pub trait Notification: GetTgMessage {}

impl Notification for EnvOutOfRangeNotification {}
impl Notification for BackendErrorNotification {}

impl GetTgMessage for EnvOutOfRangeNotification {
    fn get_tg_message(&self) -> TgMessage {
        let readings: Vec<String> = self
            .readings_out_or_range
            .iter()
            .map(|r| format!("{}", r))
            .collect();
        let readings = readings.join(", ");

        TgMessage {
            format: TgMessageFormat::MarkdownV2,
            text: format!(
                "{readings} is \\(are\\) out of normal range\n\n\
                {latest_sample_md}",
                latest_sample_md = self.latest_sample.format_as_markdown()
            ),
        }
    }
}

impl GetTgMessage for BackendErrorNotification {
    fn get_tg_message(&self) -> TgMessage {
        TgMessage {
            format: TgMessageFormat::Html,
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
