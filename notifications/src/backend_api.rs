use crate::{config::Config, sample::Sample};
use chrono::{DateTime, Utc};
use reqwest::{Response, StatusCode};
use std::{sync::Arc, time::Duration};

#[derive(thiserror::Error, Debug)]
pub enum BackendApiError {
    #[error("failed to reach Meteos API: {}", .source)]
    UnreachableApi { source: UnreachableApiError },
    #[error("failed to parse Meteos API response: {}", .source)]
    ResponseDeserializationFailed { source: reqwest::Error },
    #[error("no environment samples received from Meteos API for period from {}", .from)]
    NoEnvSamples { from: DateTime<Utc> },
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
        let backend_url = &self.config.backend_url;
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
        let from_str = from.to_rfc3339_opts(chrono::SecondsFormat::Secs, true);
        let response = self.request(&format!("samples?from={from_str}")).await?;
        let samples = response
            .json::<Vec<Sample>>()
            .await
            .map_err(|err| BackendApiError::ResponseDeserializationFailed { source: err })?;

        if samples.is_empty() {
            Err(BackendApiError::NoEnvSamples { from })
        } else {
            Ok(samples)
        }
    }

    pub async fn get_latest_samples(
        &self,
        period: Duration,
    ) -> Result<Vec<Sample>, BackendApiError> {
        let now: chrono::DateTime<chrono::Utc> = std::time::SystemTime::now().into();
        let period = chrono::Duration::from_std(period).expect("failed to parse duration");
        let from = now
            .checked_sub_signed(period)
            .expect("failed to substruct duration");

        self.get_samples(from).await
    }
}
