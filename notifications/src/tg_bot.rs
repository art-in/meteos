use crate::{
    backend_api::BackendApi,
    config::Config,
    subscriptions::{Subscriptions, TgSubscription},
};
use std::{error::Error, sync::Arc};
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
    backend_api: Arc<BackendApi>,
}

#[derive(Clone)]
struct Ctx {
    pub subs: Arc<Mutex<Subscriptions>>,
    pub backend_api: Arc<BackendApi>,
}

impl TgBot {
    pub fn new(
        subs: Arc<Mutex<Subscriptions>>,
        config: Arc<Config>,
        backend_api: Arc<BackendApi>,
    ) -> Self {
        let bot = Bot::new(config.tg_bot_token.clone()).auto_send();
        TgBot {
            bot,
            subs,
            backend_api,
        }
    }

    pub async fn send_message(&self, sub: &TgSubscription, message: &str) {
        log::debug!(
            "send_message(chat_id={chat_id}, message={message})",
            chat_id = sub.chat_id,
            message = message
        );
        self.bot
            .send_message(ChatId(sub.chat_id), message)
            .parse_mode(ParseMode::MarkdownV2)
            .await
            .unwrap();
    }

    pub async fn start_command_server(&self) {
        log::info!("starting command server...");

        let ctx = Ctx {
            subs: self.subs.clone(),
            backend_api: self.backend_api.clone(),
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
    Start,
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
        Command::Start => on_command_help(bot, message).await?,
        Command::Help => on_command_help(bot, message).await?,
        Command::Env => on_command_env(bot, message, ctx).await?,
        Command::Subscribe => on_command_subscribe(bot, message, ctx).await?,
        // TODO: add Unsubscribe
    };

    Ok(())
}

async fn on_command_help(
    bot: AutoSend<Bot>,
    message: Message,
) -> Result<(), Box<dyn Error + Send + Sync>> {
    bot.send_message(message.chat.id, Command::descriptions().to_string())
        .await?;
    Ok(())
}

async fn on_command_env(
    bot: AutoSend<Bot>,
    message: Message,
    ctx: Ctx,
) -> Result<(), Box<dyn Error + Send + Sync>> {
    let samples = ctx.backend_api.get_latest_samples().await;

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
        chat_id: message.chat.id.0,
    };

    ctx.subs.lock().await.add_tg_sub(sub).await;

    bot.send_message(message.chat.id, "You are subscribed to notifications!")
        .await?;

    Ok(())
}
