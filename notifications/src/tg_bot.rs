use crate::{
    backend_api,
    subs::{Subscriptions, TgSubscription},
};
use std::{error::Error, sync::Arc, time::Duration};
use teloxide::{
    payloads::SendMessageSetters,
    prelude::*,
    types::{ChatId, ParseMode},
    utils::command::BotCommands,
};
use tokio::sync::Mutex;

pub struct TgBot {
    bot: AutoSend<Bot>,
    subs: Arc<Mutex<Subscriptions>>,
}

#[derive(Clone)]
struct Ctx {
    pub subs: Arc<Mutex<Subscriptions>>,
}

impl TgBot {
    pub fn new(subs: Arc<Mutex<Subscriptions>>) -> Self {
        let token = std::env::var("TG_BOT_TOKEN").expect("TG_BOT_TOKEN");
        let bot = Bot::new(token).auto_send();
        TgBot { bot, subs }
    }

    pub async fn send_message(&self, sub: &TgSubscription, message: &str) {
        log::debug!("send_notification(user_id={})", sub.user_id);
        self.bot
            .send_message(ChatId(sub.user_id), message)
            .parse_mode(ParseMode::MarkdownV2)
            .await
            .unwrap();
    }

    pub async fn start_command_server(&self) {
        log::info!("starting command server...");

        let ctx = Ctx {
            subs: self.subs.clone(),
        };

        let command_handler_with_context =
            move |bot, message, command| command_handler(bot, message, command, ctx.clone());

        teloxide::commands_repl(
            self.bot.clone(),
            command_handler_with_context,
            Command::ty(),
        )
        .await;
    }
}

#[derive(BotCommands, Clone)]
#[command(rename = "lowercase", description = "These commands are supported:")]
enum Command {
    #[command(description = "show available commands")]
    Help,
    #[command(description = "show latest environment data")]
    Env,
    #[command(description = "subscribe to notifications")]
    Subscribe,
}

async fn command_handler(
    bot: AutoSend<Bot>,
    message: Message,
    command: Command,
    ctx: Ctx,
) -> Result<(), Box<dyn Error + Send + Sync>> {
    match command {
        // TODO: add /start, with same response as on /help
        Command::Help => {
            bot.send_message(message.chat.id, Command::descriptions().to_string())
                .await?;
        }
        Command::Env => on_command_env(bot, message).await?,
        Command::Subscribe => on_command_subscribe(bot, message, ctx).await?,
        // TODO: add Unsubscribe
    };

    Ok(())
}

async fn on_command_env(
    bot: AutoSend<Bot>,
    message: Message,
) -> Result<(), Box<dyn Error + Send + Sync>> {
    let samples = backend_api::get_latest_samples(Duration::from_secs(300)).await;

    let response_text = match samples {
        Ok(samples) => format!("{}", samples[samples.len() - 1]),
        Err(error) => format!("Error: {}", error),
    };

    bot.send_message(message.chat.id, response_text)
        .parse_mode(ParseMode::MarkdownV2)
        .await?;

    Ok(())
}

async fn on_command_subscribe(
    bot: AutoSend<Bot>,
    message: Message,
    ctx: Ctx,
) -> Result<(), Box<dyn Error + Send + Sync>> {
    let sub = TgSubscription {
        user_id: message.chat.id.0,
    };
    let mut subs = ctx.subs.lock().await;
    subs.add_tg_sub(sub).await;

    bot.send_message(message.chat.id, "You are subscribed to notifications!")
        .await?;

    Ok(())
}
