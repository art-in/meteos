use crate::{
    backend_api::{BackendApi, Reading},
    config::Config,
    notification::{BackendErrorNotification, NotOptimalEnviromentalReadingsNotification},
    notifier::Notifier,
};
use anyhow::Result;
use std::{
    sync::Arc,
    time::{Duration, Instant},
};

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
    let mut consecutive_errors: Option<ConsecutiveErrors> = None;
    let mut is_out_of_range_notification_sent = false;

    loop {
        log::debug!("new check loop iteration");
        let latest_samples = backend_api.get_latest_samples(config.check_period).await;

        log::trace!("received samples: {:?}", latest_samples);

        match latest_samples {
            Ok(samples) => {
                consecutive_errors = None;

                let mut readings_out_of_range = Vec::new();

                // only broadcast notification if certain reading is out of optimal range in all
                // samples of check period. this should smooth short spikes of certain reading.
                // eg. when co2 goes beyond range max for one minute when human is too close to
                // the sensor, we don't want to raise alarm for that
                if samples
                    .iter()
                    .all(|s| !config.optimal_ranges.temperature.contains(&s.temperature))
                {
                    readings_out_of_range.push(Reading::Temperature);
                }

                if samples
                    .iter()
                    .all(|s| !config.optimal_ranges.humidity.contains(&s.humidity))
                {
                    readings_out_of_range.push(Reading::Humidity);
                }

                if samples
                    .iter()
                    .all(|s| !config.optimal_ranges.co2.contains(&s.co2))
                {
                    readings_out_of_range.push(Reading::Co2);
                }

                if samples
                    .iter()
                    .all(|s| !config.optimal_ranges.pressure.contains(&s.pressure))
                {
                    readings_out_of_range.push(Reading::Pressure);
                }

                if !readings_out_of_range.is_empty() {
                    if !is_out_of_range_notification_sent {
                        notifier
                            .broadcast(Box::new(NotOptimalEnviromentalReadingsNotification {
                                readings_out_of_range,
                                latest_sample: samples[samples.len() - 1].clone(),
                            }))
                            .await?;
                        is_out_of_range_notification_sent = true;
                    }
                } else {
                    is_out_of_range_notification_sent = false;
                }
            }
            Err(error) => {
                is_out_of_range_notification_sent = false;
                log::trace!("{:?}", consecutive_errors);
                log::error!("{}", error);

                // do not broadcast error notification immediately after first error, give it some
                // time and broadcast only if it consistently failing for some period of time
                match consecutive_errors.as_mut() {
                    None => {
                        consecutive_errors = Some(ConsecutiveErrors {
                            first_error_time: Instant::now(),
                            error_count: 0,
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
