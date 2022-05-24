// TODO: warn on ".unwrap()"
// TODO: add scripts for publishing to docker

use notifier::Notifier;
use std::sync::Arc;

mod backend_api;
mod check;
mod notification;
mod notifier;
mod subs;
mod tg_bot;

#[tokio::main]
async fn main() {
    dotenv::dotenv().unwrap();
    pretty_env_logger::init();

    let notifier = Arc::new(Notifier::new());
    let notifier_clone = notifier.clone();

    let h1 = tokio::spawn(async move {
        notifier.start_subscription_service().await;
    });

    let h2 = tokio::spawn(async move {
        check::start(notifier_clone).await;
    });

    h1.await.unwrap();
    h2.await.unwrap();
}
