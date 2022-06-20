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
enum State {
    AllGood,
    NotOptimalReadings(Vec<ReadingOptimality>),
    BackendErrors {
        notification_broadcasted: bool,
        first_error_time: Instant,
        error_count: u32,
    },
}

pub async fn start(
    notifier: Arc<Notifier>,
    config: Arc<Config>,
    backend_api: Arc<BackendApi>,
) -> Result<()> {
    log::debug!("starting monitor loop...");

    let mut state = State::AllGood;

    loop {
        log::trace!("next iteration");
        let latest_samples = backend_api
            .get_latest_samples(config.monitoring_period)
            .await;

        state = match latest_samples {
            Ok(samples) => {
                log::trace!("received samples: {:?}", samples);

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
                // monitoring period. this should smooth short-time spikes
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

                    // re-notify about new not optimal readings.
                    // eg. pressure can be not optimal for days, but we still want to notify if co2
                    // or temperature go out of optimal range in that period
                    let has_new_not_optimal_readings =
                        if let State::NotOptimalReadings(previous_not_optimal_readings) = state {
                            not_optimal_readings
                                .iter()
                                .any(|r| previous_not_optimal_readings.iter().all(|pr| r != pr))
                        } else {
                            true
                        };

                    if has_new_not_optimal_readings {
                        notifier
                            .broadcast(Box::new(NotOptimalReadingsNotification {
                                not_optimal_readings: not_optimal_readings.clone(),
                                latest_sample: samples[samples.len() - 1].clone(),
                                optimal_ranges: config.optimal_ranges.clone(),
                            }))
                            .await?;
                    }

                    State::NotOptimalReadings(not_optimal_readings)
                } else {
                    log::trace!("all readings are optimal");
                    State::AllGood
                }
            }
            Err(error) => {
                log::error!("backend error: {}", error);

                // do not broadcast error notification immediately after first error, give it some
                // time and broadcast only if it consistently failing for some period of time
                match state {
                    State::BackendErrors {
                        mut notification_broadcasted,
                        first_error_time,
                        mut error_count,
                    } => {
                        if !notification_broadcasted {
                            let error_period = Instant::now().duration_since(first_error_time);

                            error_count += 1;

                            if error_period >= config.backend_error_timeout {
                                notifier
                                    .broadcast(Box::new(BackendErrorNotification {
                                        latest_error: error,
                                        error_period,
                                        error_count,
                                    }))
                                    .await?;
                                notification_broadcasted = true;
                            }
                        }

                        State::BackendErrors {
                            notification_broadcasted,
                            first_error_time,
                            error_count,
                        }
                    }
                    _ => State::BackendErrors {
                        first_error_time: Instant::now(),
                        error_count: 1,
                        notification_broadcasted: false,
                    },
                }
            }
        };

        tokio::time::sleep(config.monitoring_interval).await;
    }
}
