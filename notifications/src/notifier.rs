use crate::{
    backend_api::BackendApi, config::Config, notification::Notification,
    subscriptions::Subscriptions, tg_bot::TgBot,
};
use std::sync::Arc;
use tokio::sync::Mutex;

pub struct Notifier {
    subs: Arc<Mutex<Subscriptions>>,
    tg_bot: TgBot,
}

impl Notifier {
    pub fn new(config: Arc<Config>, backend_api: Arc<BackendApi>) -> Self {
        let subs = Arc::new(Mutex::new(Subscriptions::load()));
        let tg_bot = TgBot::new(subs.clone(), config, backend_api);
        Notifier { subs, tg_bot }
    }

    pub async fn start_subscription_service(&self) {
        self.tg_bot.start_command_server().await;
    }

    pub async fn broadcast(&self, notification: Box<dyn Notification + Send + Sync>) {
        let subs = self.subs.lock().await;
        let subs = subs.get_tg_subs();

        log::debug!(
            "Notifier::broadcast(subs_count={subs_count}, notification={notification:?})",
            subs_count = subs.len(),
            notification = notification
        );

        for sub in &subs {
            self.tg_bot
                .send_message(sub, &notification.get_message())
                .await;
        }
    }
}
