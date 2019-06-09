import {useState, useRef, useCallback} from 'react';

import log from 'utils/log';

const SERVICE_UUID = 'b8acbbea-21c2-4c6a-a52f-6033f2f6d1a6';
const CHAR_UUID_WIFI_SSID = 'd1fe36b7-3223-4927-ada5-422f2b5afcf9';
const CHAR_UUID_WIFI_PASS = '5e527e75-2d5a-40f1-95fb-0e4a7d612ee9';
const CHAR_UUID_BACKEND_HOST = '8ebaa047-47d0-4765-a068-fb96a3b6d6b9';
const CHAR_UUID_BACKEND_PORT = '4bb0157c-1c74-4cfb-81d8-351b895ce811';
const CHAR_UUID_SAMPLE_DELAY = 'a8222441-cb54-46d6-8765-173b3b1e06e2';

/**
 * Provides access to Sensor configuration service over bluetooth.
 */
export default function useConfigurator() {
  const [isConnected, setIsConnected] = useState(false);
  const [isConnecting, setIsConnecting] = useState(false);
  const [config, setConfig] = useState({});
  const [isUploadingConfig, setIsUploadingConfig] = useState(false);

  const charsRef = useRef();
  const deviceRef = useRef();
  const serverRef = useRef();

  const onGATTServerDisconnected = useCallback(() => {
    log('GATT server disconnected');
    setIsConnected(false);
  }, []);

  const connect = useCallback(async () => {
    log('requesting bluetooth device...');

    let device;

    try {
      device = await navigator.bluetooth.requestDevice({
        filters: [{services: [SERVICE_UUID]}]
      });
    } catch (e) {
      if (e.message.startsWith('User cancelled')) {
        // user canceled bluetooth pairing dialog
        return;
      }

      alert(`Failed to request bluetooth device:\n${e.message}`);
      throw e;
    }

    setIsConnecting(true);

    deviceRef.current = device;
    device.addEventListener('gattserverdisconnected', onGATTServerDisconnected);

    log('connecting to GATT Server...');
    const server = await device.gatt.connect();
    serverRef.current = server;

    log('getting service...');
    const service = await server.getPrimaryService(SERVICE_UUID);

    log('getting characteristics...');
    var dec = new TextDecoder(); // utf-8
    const chars = {};

    chars.wifiSSID = await service.getCharacteristic(CHAR_UUID_WIFI_SSID);
    chars.wifiPass = await service.getCharacteristic(CHAR_UUID_WIFI_PASS);
    chars.backendHost = await service.getCharacteristic(CHAR_UUID_BACKEND_HOST);
    chars.backendPort = await service.getCharacteristic(CHAR_UUID_BACKEND_PORT);
    chars.sampleDelay = await service.getCharacteristic(CHAR_UUID_SAMPLE_DELAY);

    charsRef.current = chars;

    log('reading characteristics...');
    const wifiSSID = dec.decode(await chars.wifiSSID.readValue());
    const wifiPass = dec.decode(await chars.wifiPass.readValue());
    const backendHost = dec.decode(await chars.backendHost.readValue());
    const backendPort = dec.decode(await chars.backendPort.readValue());
    const sampleDelay = dec.decode(await chars.sampleDelay.readValue());

    setConfig({
      wifiSSID,
      wifiPass,
      backendHost,
      backendPort,
      sampleDelay
    });

    log('characteristics read.');

    setIsConnecting(false);
    setIsConnected(true);
  }, [onGATTServerDisconnected]);

  const disconnect = useCallback(() => {
    const device = deviceRef.current;
    const server = serverRef.current;

    device.removeEventListener(
      'gattserverdisconnected',
      onGATTServerDisconnected
    );
    server.disconnect();

    log('disconnected from GATT server.');
  }, [onGATTServerDisconnected]);

  const uploadConfig = useCallback(
    async (wifiSSID, wifiPass, backendHost, backendPort, sampleDelay) => {
      log('writing to characteristics...');
      setIsUploadingConfig(true);

      try {
        var enc = new TextEncoder(); // utf-8

        const chars = charsRef.current;

        await chars.wifiSSID.writeValue(enc.encode(wifiSSID));
        await chars.wifiPass.writeValue(enc.encode(wifiPass));
        await chars.backendHost.writeValue(enc.encode(backendHost));
        await chars.backendPort.writeValue(enc.encode(backendPort));
        await chars.sampleDelay.writeValue(enc.encode(sampleDelay));

        log('characteristics written.');
      } catch (e) {
        alert(`Failed to write service characteristics:\n${e.message}`);
        throw e;
      } finally {
        setIsUploadingConfig(false);
      }
    },
    []
  );

  return {
    isConnected,
    isConnecting,
    connect,
    disconnect,

    config,
    uploadConfig,
    isUploadingConfig
  };
}
