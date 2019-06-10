import moment from 'moment';

import {getTime} from './api';
import log from './log';

const SYNC_PERIOD = 30 * 60 * 1000; // 30 mins

// re-sync periodically and not just once at startup, because if started in
// offline we need to sync when server connection appears anyway.
export default async function startSyncTimeWithServer() {
  await syncTimeWithServer();
  setTimeout(startSyncTimeWithServer, SYNC_PERIOD);
}

async function syncTimeWithServer() {
  let serverTime;

  try {
    serverTime = moment(await getTime());
  } catch (e) {
    log(`failed to get server time. skipping sync.`);
    return;
  }

  const clientTime = moment();
  const offset = clientTime - serverTime;

  moment.now = () => {
    return +new Date() - offset;
  };

  log(`synced time with server (offset ${offset} ms)`);
}
