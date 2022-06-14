use crate::{
    backend_api::BackendApi,
    config::Config,
    notification::{BackendErrorNotification, NotOptimalReadingsNotification},
    notifier::Notifier,
    reading::{get_all_readings_optimality, Optimality, Reading, ReadingOptimality},
};
use anyhow::Result;
use std::{sync::Arc, time::Instant};

#[derive(Debug)]
struct ConsecutiveErrors {
    pub first_error_time: Instant,
    pub error_count: u32,
    pub notification_broadcasted: bool,
}

pub async fn start(
    notifier: Arc<Notifier>,
    config: Arc<Config>,
    backend_api: Arc<BackendApi>,
) -> Result<()> {
    log::debug!("starting check loop...");

    let mut consecutive_errors: Option<ConsecutiveErrors> = None;
    let mut is_not_optimal_readings_notification_sent = false;

    loop {
        log::trace!("next iteration");
        let latest_samples = backend_api.get_latest_samples(config.check_period).await;

        match latest_samples {
            Ok(samples) => {
                log::trace!("received samples: {:?}", samples);

                consecutive_errors = None;

                let readings_optimality = vec![
                    ReadingOptimality {
                        reading: Reading::Temperature,
                        optimality: get_all_readings_optimality(
                            samples.iter().map(|s| s.temperature).collect(),
                            &config.optimal_ranges.temperature,
                        ),
                    },
                    ReadingOptimality {
                        reading: Reading::Humidity,
                        optimality: get_all_readings_optimality(
                            samples.iter().map(|s| s.humidity).collect(),
                            &config.optimal_ranges.humidity,
                        ),
                    },
                    ReadingOptimality {
                        reading: Reading::Pressure,
                        optimality: get_all_readings_optimality(
                            samples.iter().map(|s| s.pressure).collect(),
                            &config.optimal_ranges.pressure,
                        ),
                    },
                    ReadingOptimality {
                        reading: Reading::Co2,
                        optimality: get_all_readings_optimality(
                            samples.iter().map(|s| s.co2).collect(),
                            &config.optimal_ranges.co2,
                        ),
                    },
                ];

                // only broadcast notification if certain reading is not optimal in all samples of
                // check period. this should smooth short-time spikes
                // eg. when co2 goes beyond range max for one minute when human is too close to
                // the sensor, we don't want to raise alarm for that
                let not_optimal_readings: Vec<ReadingOptimality> = readings_optimality
                    .iter()
                    .copied()
                    .filter(|ro| {
                        ro.optimality != Optimality::Optimal && ro.optimality != Optimality::Mixed
                    })
                    .collect();

                if !not_optimal_readings.is_empty() {
                    log::trace!("not optimal readings: {:?}", not_optimal_readings);
                    if !is_not_optimal_readings_notification_sent {
                        notifier
                            .broadcast(Box::new(NotOptimalReadingsNotification {
                                not_optimal_readings,
                                latest_sample: samples[samples.len() - 1].clone(),
                                optimal_ranges: config.optimal_ranges.clone(),
                            }))
                            .await?;
                        is_not_optimal_readings_notification_sent = true;
                    }
                } else {
                    log::trace!("all readings are optimal");
                    is_not_optimal_readings_notification_sent = false;
                }
            }
            Err(error) => {
                log::error!("backend error: {}", error);
                log::trace!("previous errors: {:?}", consecutive_errors);

                is_not_optimal_readings_notification_sent = false;

                // do not broadcast error notification immediately after first error, give it some
                // time and broadcast only if it consistently failing for some period of time
                match consecutive_errors.as_mut() {
                    None => {
                        consecutive_errors = Some(ConsecutiveErrors {
                            first_error_time: Instant::now(),
                            error_count: 1,
                            notification_broadcasted: false,
                        });
                    }
                    Some(consecutive_errors) => {
                        if !consecutive_errors.notification_broadcasted {
                            let error_period =
                                Instant::now().duration_since(consecutive_errors.first_error_time);

                            consecutive_errors.error_count += 1;

                            if error_period >= config.backend_error_timeout {
                                notifier
                                    .broadcast(Box::new(BackendErrorNotification {
                                        latest_error: error,
                                        error_period,
                                        error_count: consecutive_errors.error_count,
                                    }))
                                    .await?;
                                consecutive_errors.notification_broadcasted = true;
                            }
                        }
                    }
                }
            }
        }

        tokio::time::sleep(config.check_interval).await;
    }
}
