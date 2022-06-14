use crate::{
    backend_api::BackendApi,
    config::Config,
    subscriptions::{Subscriptions, TgChat, TgChatPrivate, TgChatPublic, TgSubscription},
};
use anyhow::{Context, Result};
use std::{fmt::Debug, sync::Arc};
use teloxide::{
    adaptors::AutoSend,
    payloads::SendMessageSetters,
    prelude::{Requester, RequesterExt},
    types::{ChatId, ChatKind, ParseMode},
    utils::command::BotCommands,
    Bot,
};

pub struct TgBot {
    bot: AutoSend<Bot>,
    subs: Arc<Subscriptions>,
    backend_api: Arc<BackendApi>,
    config: Arc<Config>,
}

pub struct TgMessage {
    pub format: TgMessageFormat,
    pub text: String,
}

pub enum TgMessageFormat {
    Html,
    MarkdownV2,
}

pub trait GetTgMessage: Debug {
    fn get_tg_message(&self) -> TgMessage;
}
#[derive(Clone)]
struct CommandHandlerContext {
    pub subs: Arc<Subscriptions>,
    pub backend_api: Arc<BackendApi>,
    pub config: Arc<Config>,
}

impl TgBot {
    pub fn new(
        subs: Arc<Subscriptions>,
        config: Arc<Config>,
        backend_api: Arc<BackendApi>,
    ) -> Self {
        let bot = Bot::new(config.tg_bot_token.clone()).auto_send();
        TgBot {
            bot,
            subs,
            backend_api,
            config,
        }
    }

    pub async fn start_command_server(&self) {
        log::debug!("starting command server...");

        let ctx = CommandHandlerContext {
            subs: self.subs.clone(),
            backend_api: self.backend_api.clone(),
            config: self.config.clone(),
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

    pub async fn send_message(&self, chat_id: i64, message: TgMessage) -> Result<()> {
        send_message_impl(&self.bot, chat_id, message).await
    }
}

async fn send_message_impl(bot: &AutoSend<Bot>, chat_id: i64, message: TgMessage) -> Result<()> {
    log::trace!(
        "send_message: chat_id={chat_id}, message=\n{message}",
        chat_id = chat_id,
        message = message.text
    );

    bot.send_message(ChatId(chat_id), &message.text)
        .parse_mode(match message.format {
            TgMessageFormat::Html => ParseMode::Html,
            TgMessageFormat::MarkdownV2 => ParseMode::MarkdownV2,
        })
        .await
        .context(format!(
            "failed to send telegram message: \"{message}\"",
            message = &message.text,
        ))?;

    Ok(())
}

#[derive(BotCommands, Clone)]
#[command(rename = "lowercase", description = "These commands are supported:")]
enum Command {
    Start,
    #[command(description = "show available commands")]
    Help,
    #[command(description = "show latest environment sample", rename = "snake_case")]
    LatestSample,
    #[command(description = "subscribe to notifications")]
    Subscribe,
    #[command(description = "unsubscribe from notifications")]
    Unsubscribe,
}

async fn command_handler(
    bot: AutoSend<Bot>,
    message: teloxide::types::Message,
    command: Command,
    ctx: CommandHandlerContext,
) -> Result<()> {
    match command {
        Command::Start => on_command_help(bot, message).await?,
        Command::Help => on_command_help(bot, message).await?,
        Command::LatestSample => on_command_latest_sample(bot, message, ctx).await?,
        Command::Subscribe => on_command_subscribe(bot, message, ctx).await?,
        Command::Unsubscribe => on_command_unsubscribe(bot, message, ctx).await?,
    };

    Ok(())
}

async fn on_command_help(bot: AutoSend<Bot>, message: teloxide::types::Message) -> Result<()> {
    log::trace!("on_command_help: message={:?}", message);
    send_message_impl(
        &bot,
        message.chat.id.0,
        TgMessage {
            format: TgMessageFormat::Html,
            text: Command::descriptions().to_string(),
        },
    )
    .await?;
    Ok(())
}

async fn on_command_latest_sample(
    bot: AutoSend<Bot>,
    message: teloxide::types::Message,
    ctx: CommandHandlerContext,
) -> Result<()> {
    log::trace!("on_command_latest_sample: message={:?}", message);
    let latest_samples = ctx
        .backend_api
        .get_latest_samples(ctx.config.check_period)
        .await;

    let outgoing_message = match latest_samples {
        Ok(samples) => TgMessage {
            format: TgMessageFormat::MarkdownV2,
            text: samples[samples.len() - 1].format_as_markdown(&ctx.config.optimal_ranges),
        },
        Err(error) => {
            log::error!("{}", error);
            TgMessage {
                format: TgMessageFormat::Html,
                text: format!("ERROR: {}", error),
            }
        }
    };

    send_message_impl(&bot, message.chat.id.0, outgoing_message).await?;

    Ok(())
}

async fn on_command_subscribe(
    bot: AutoSend<Bot>,
    message: teloxide::types::Message,
    ctx: CommandHandlerContext,
) -> Result<()> {
    log::trace!("on_command_subscribe: message={:?}", message);
    let sub = TgSubscription {
        chat_id: message.chat.id.0,
        chat_info: match message.chat.kind {
            ChatKind::Private(chat) => TgChat::Private(TgChatPrivate {
                username: chat.username,
                first_name: chat.first_name,
                last_name: chat.last_name,
            }),
            ChatKind::Public(chat) => TgChat::Public(TgChatPublic {
                title: chat.title,
                description: chat.description,
            }),
        },
    };

    let is_new_sub = ctx
        .subs
        .add_tg_sub(sub)
        .context("failed to add telegram subscription")?;

    let outgoing_message = TgMessage {
        format: TgMessageFormat::Html,
        text: if is_new_sub {
            "You are subscribed to notifications!".into()
        } else {
            "You was already subscribed before.".into()
        },
    };

    send_message_impl(&bot, message.chat.id.0, outgoing_message).await?;

    Ok(())
}

async fn on_command_unsubscribe(
    bot: AutoSend<Bot>,
    message: teloxide::types::Message,
    ctx: CommandHandlerContext,
) -> Result<()> {
    log::trace!("on_command_unsubscribe: message={:?}", message);
    let was_existing_sub = ctx
        .subs
        .remove_tg_sub(message.chat.id.0)
        .context("failed to remove telegram subscription")?;

    let outgoing_message = TgMessage {
        format: TgMessageFormat::Html,
        text: if was_existing_sub {
            "You was unsubscribed from notifications!".into()
        } else {
            "You are not subscribed yet.".into()
        },
    };

    send_message_impl(&bot, message.chat.id.0, outgoing_message).await?;

    Ok(())
}
