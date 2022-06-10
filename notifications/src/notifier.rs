use crate::{
    backend_api::BackendApi, config::Config, notification::Notification,
    subscriptions::Subscriptions, tg_bot::TgBot,
};
use anyhow::{Context, Result};
use std::sync::Arc;
use tokio::sync::Mutex;

pub struct Notifier {
    subs: Arc<Mutex<Subscriptions>>,
    tg_bot: TgBot,
}

impl Notifier {
    pub fn init(config: Arc<Config>, backend_api: Arc<BackendApi>) -> Result<Self> {
        let subs = Arc::new(Mutex::new(
            Subscriptions::load().context("failed to load subscriptions")?,
        ));
        let tg_bot = TgBot::new(subs.clone(), config, backend_api);
        Ok(Notifier { subs, tg_bot })
    }

    pub async fn start_subscription_service(&self) {
        self.tg_bot.start_command_server().await;
    }

    pub async fn broadcast(&self, notification: Box<dyn Notification + Send + Sync>) -> Result<()> {
        let subs = self.subs.lock().await;
        let subs = subs
            .get_tg_subs()
            .context("failed to get telegram subscriptions")?;

        log::debug!(
            "Notifier::broadcast(subs_count={subs_count}, notification={notification:?})",
            subs_count = subs.len(),
            notification = notification
        );

        for sub in &subs {
            let res = self
                .tg_bot
                .send_message(sub.chat_id, notification.get_tg_message())
                .await;

            if let Err(error) = res {
                log::error!(
                    "failed to send notification to telegram subscription {sub:?}: {error:?}",
                    sub = sub,
                    error = error
                );
            }
        }

        Ok(())
    }
}
