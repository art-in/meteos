// TODO: warn on ".unwrap()"
// TODO: add scripts for publishing to docker

use crate::{backend_api::BackendApi, config::Config};
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
async fn main() {
    pretty_env_logger::init();

    let config = Arc::new(Config::read().expect("failed to read config"));
    let backend_api = Arc::new(BackendApi::new(config.clone()));

    let notifier = Arc::new(Notifier::new(config.clone(), backend_api.clone()));
    let notifier_clone = notifier.clone();

    let subscription_service_task = async move {
        notifier.start_subscription_service().await;
    };

    let env_check_task = async move {
        env_check::start(notifier_clone, config, backend_api).await;
    };

    tokio::join!(subscription_service_task, env_check_task);
}
