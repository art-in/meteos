#![warn(clippy::unwrap_used)]

use crate::{backend_api::BackendApi, config::Config};
use anyhow::{Context, Result};
use notifier::Notifier;
use simple_logger::SimpleLogger;
use std::sync::Arc;

mod backend_api;
mod check;
mod config;
mod notification;
mod notifier;
mod reading;
mod sample;
mod subscriptions;
mod tg_bot;
mod utils;

#[tokio::main]
async fn main() -> Result<()> {
    let config = Arc::new(Config::read().context("failed to read config")?);

    SimpleLogger::new()
        .with_level(log::LevelFilter::Info)
        .with_module_level("meteos", config.log_level)
        .init()
        .expect("failed to init logger");

    log::info!("starting service: config={:?}", config);

    let backend_api = Arc::new(BackendApi::new(config.clone()));
    let notifier = Arc::new(
        Notifier::init(config.clone(), backend_api.clone()).context("failed to init notifier")?,
    );

    let subscription_task = {
        let notifier = notifier.clone();
        async move {
            notifier.start_subscription_service().await;
        }
    };

    let check_task = {
        let notifier = notifier.clone();
        async move {
            check::start(notifier, config, backend_api)
                .await
                .expect("failed to run environment check service");
        }
    };

    tokio::join!(subscription_task, check_task);

    Ok(())
}
