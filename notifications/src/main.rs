#![warn(clippy::unwrap_used)]

// TODO: add scripts for publishing to docker
// TODO: refactor log messages

use crate::{backend_api::BackendApi, config::Config};
use anyhow::{Context, Result};
use notifier::Notifier;
use std::sync::Arc;

mod backend_api;
mod config;
mod env_check;
mod notification;
mod notifier;
mod subscriptions;
mod tg_bot;

#[tokio::main]
async fn main() -> Result<()> {
    pretty_env_logger::init();

    let config = Arc::new(Config::read().context("failed to read config")?);
    let backend_api = Arc::new(BackendApi::new(config.clone()));

    let notifier = Arc::new(
        Notifier::init(config.clone(), backend_api.clone()).context("failed to init notifier")?,
    );
    let notifier_clone = notifier.clone();

    let subscription_service_task = async move {
        notifier.start_subscription_service().await;
    };

    let env_check_task = async move {
        env_check::start(notifier_clone, config, backend_api)
            .await
            .expect("failed to run environment check service");
    };

    tokio::join!(subscription_service_task, env_check_task);

    Ok(())
}
