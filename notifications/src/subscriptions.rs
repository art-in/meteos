use anyhow::{Context, Result};
use rustbreak::{backend::FileBackend, deser::Ron, Database, FileDatabase};
use serde::{Deserialize, Serialize};
use std::{collections::HashMap, fs::create_dir_all};

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct TgSubscription {
    pub chat_id: i64,

    // chat info saved for debug purposes only for now.
    // not saving teloxide::ChatKind directly because it fails on deserialization for some reason
    pub chat_info: TgChat,
}

#[derive(Clone, Debug, Serialize, Deserialize)]
pub enum TgChat {
    Private(TgChatPrivate),
    Public(TgChatPublic),
}

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct TgChatPrivate {
    pub username: Option<String>,
    pub first_name: Option<String>,
    pub last_name: Option<String>,
}

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct TgChatPublic {
    pub title: Option<String>,
    pub description: Option<String>,
}

#[derive(Clone, Default, Serialize, Deserialize)]
struct DbData {
    tg_subs: HashMap<i64, TgSubscription>,
}

pub struct Subscriptions {
    db: Database<DbData, FileBackend, Ron>,
}

impl Subscriptions {
    pub fn load() -> Result<Self> {
        create_dir_all("db").context("failed to create database directory")?;

        let db = FileDatabase::<DbData, Ron>::load_from_path_or_else("db/db.ron", DbData::default)
            .context("failed to load database from file")?;

        Ok(Subscriptions { db })
    }

    pub fn add_tg_sub(&mut self, sub: TgSubscription) -> Result<bool> {
        let is_new_sub = self
            .db
            .write(move |data| data.tg_subs.insert(sub.chat_id, sub).is_none())
            .context("failed to write to database")?;

        self.db.save().context("failed to save database")?;

        Ok(is_new_sub)
    }

    pub fn remove_tg_sub(&mut self, chat_id: i64) -> Result<bool> {
        let was_existing_sub = self
            .db
            .write(|db| db.tg_subs.remove(&chat_id).is_some())
            .context("failed to write to database")?;

        self.db.save().context("failed to save database")?;

        Ok(was_existing_sub)
    }

    pub fn get_tg_subs(&self) -> Result<Vec<TgSubscription>> {
        Ok(self.db.read(|db| db.tg_subs.values().cloned().collect())?)
    }
}
