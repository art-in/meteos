pub struct TgSubscription {
    pub user_id: i64,
}

pub struct Subscriptions {
    tg_subs: Vec<TgSubscription>,
}

impl Subscriptions {
    pub fn new() -> Self {
        Subscriptions {
            // TODO: load subscriptions from database
            tg_subs: Vec::new(),
        }
    }

    pub async fn add_tg_sub(&mut self, sub: TgSubscription) {
        // TODO: save subscription to database
        self.tg_subs.push(sub);
    }

    pub fn get_tg_subs(&self) -> &Vec<TgSubscription> {
        &self.tg_subs
    }
}
