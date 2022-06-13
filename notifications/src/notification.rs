use crate::{
    backend_api,
    config::EnvironmentalReadingRanges,
    reading::ReadingOptimality,
    sample::Sample,
    tg_bot::{GetTgMessage, TgMessage, TgMessageFormat},
    utils::beautiful_string_join,
};
use std::{fmt::Debug, time::Duration};

#[derive(Debug)]
pub struct NotOptimalEnvironmentalReadingsNotification {
    pub not_optimal_readings: Vec<ReadingOptimality>,
    pub latest_sample: Sample,
    pub optimal_ranges: EnvironmentalReadingRanges,
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

impl Notification for NotOptimalEnvironmentalReadingsNotification {}
impl Notification for BackendErrorNotification {}

impl GetTgMessage for NotOptimalEnvironmentalReadingsNotification {
    fn get_tg_message(&self) -> TgMessage {
        let reading_statuses: Vec<&str> = self
            .not_optimal_readings
            .iter()
            .map(|r| r.get_reading_status_string())
            .collect();

        TgMessage {
            format: TgMessageFormat::MarkdownV2,
            text: format!(
                "{status}\n\n\
                {latest_sample_md}",
                status = beautiful_string_join(reading_statuses),
                latest_sample_md = self.latest_sample.format_as_markdown(&self.optimal_ranges)
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
