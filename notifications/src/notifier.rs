use crate::{notification::Notification, subs::Subscriptions, tg_bot::TgBot};
use std::sync::Arc;
use tokio::sync::Mutex;

pub struct Notifier {
    subs: Arc<Mutex<Subscriptions>>,
    tg_bot: TgBot,
}

impl Notifier {
    pub fn new() -> Self {
        let subs = Arc::new(Mutex::new(Subscriptions::new()));
        let tg_bot = TgBot::new(subs.clone());
        Notifier { subs, tg_bot }
    }

    pub async fn broadcast_notification(&self, notification: Box<dyn Notification + Send + Sync>) {
        let subs = self.subs.lock().await;
        let subs = subs.get_tg_subs();
        log::debug!(
            "Notifier::send_notification(subs_count={subs_count}, notification={notification:?})",
            subs_count = subs.len(),
            notification = notification
        );
        for sub in subs {
            self.tg_bot
                .send_message(sub, &notification.get_message())
                .await;
        }
    }

    pub async fn start_subscription_service(&self) {
        self.tg_bot.start_command_server().await;
    }
}
