use crate::{
    backend_api::{BackendApi, Reading},
    config::Config,
    notification::{BackendErrorNotification, EnvOutOfRangeNotification},
    notifier::Notifier,
};
use std::{
    sync::Arc,
    time::{Duration, SystemTime},
};

#[derive(Debug)]
struct ConsecutiveErrors {
    pub first_error_time: SystemTime,
    pub error_count: u32,
    pub notification_broadcasted: bool,
}

pub async fn start(notifier: Arc<Notifier>, config: Arc<Config>, backend_api: Arc<BackendApi>) {
    let check_interval = Duration::from_secs(config.check_interval_sec);
    let backend_error_timeout = Duration::from_secs(config.backend_error_timeout_sec);

    let mut consecutive_errors: Option<ConsecutiveErrors> = None;

    loop {
        log::debug!("new check loop iteration");
        let samples = backend_api.get_latest_samples().await;

        log::trace!("received samples: {:?}", samples);

        match samples {
            Ok(samples) => {
                consecutive_errors = None;

                // only broadcast notification if certain reading is out of normal range in all
                // samples of check period. this should smooth short spikes of certain reading.
                // eg. when co2 goes beyond range max for one minute when human is too close to
                // the sensor, we don't want to raise alarm for that
                let last_sample = &samples[samples.len() - 1];
                let mut readings_out_or_range = Vec::new();

                // TODO: move ranges to config
                if samples
                    .iter()
                    .all(|s| s.temperature < 20.0 || s.temperature > 25.0)
                {
                    readings_out_or_range.push(Reading::Temperature);
                }

                if samples
                    .iter()
                    .all(|s| s.humidity < 30.0 || s.humidity > 60.0)
                {
                    readings_out_or_range.push(Reading::Humidity);
                }

                if samples.iter().all(|s| s.co2 > 900.0) {
                    readings_out_or_range.push(Reading::Co2);
                }

                // TODO: check pressure
                // TODO: do not send notification if it was already sent

                if !readings_out_or_range.is_empty() {
                    notifier
                        .broadcast(Box::new(EnvOutOfRangeNotification {
                            readings_out_or_range,
                            last_sample: last_sample.clone(),
                        }))
                        .await;
                }
            }
            Err(error) => {
                log::trace!("{:?}", consecutive_errors);

                // do not broadcast error notification immediately after first error, give it some
                // time and broadcast only if it consistently failing for some period of time
                match consecutive_errors.as_mut() {
                    None => {
                        consecutive_errors = Some(ConsecutiveErrors {
                            first_error_time: SystemTime::now(),
                            error_count: 0,
                            notification_broadcasted: false,
                        });
                    }
                    Some(consecutive_errors) => {
                        if !consecutive_errors.notification_broadcasted {
                            let error_period = SystemTime::now()
                                .duration_since(consecutive_errors.first_error_time)
                                .unwrap();

                            consecutive_errors.error_count += 1;

                            if error_period >= backend_error_timeout {
                                notifier
                                    .broadcast(Box::new(BackendErrorNotification {
                                        last_error: error,
                                        error_period,
                                        error_count: consecutive_errors.error_count,
                                    }))
                                    .await;
                                consecutive_errors.notification_broadcasted = true;
                            }
                        }
                    }
                }
            }
        }

        tokio::time::sleep(check_interval).await;
    }
}
