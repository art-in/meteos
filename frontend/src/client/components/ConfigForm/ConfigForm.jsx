import React, {useRef, useCallback, useEffect} from 'react';
import cn from 'classnames';

import useConfigurator from 'hooks/use-configurator';
import ConnectIcon from './icons/connect.svg';
import UploadIcon from './icons/upload.svg';
import classes from './ConfigForm.css';

const CONNECT_BUTTON_TITLE =
  'Connect to Sensor over bluetooth (device should be in config mode)';
const CONNECT_BUTTON_TITLE_DONE = 'Connected to Sensor';
const UPLOAD_BUTTON_TITLE = 'Upload configuration to Sensor';

export default function ConfigForm() {
  const {
    isConnected,
    isConnecting,
    connect,
    disconnect,
    config,
    uploadConfig,
    isUploadingConfig
  } = useConfigurator();

  const formRef = useRef();
  const inputRef_wifiSSID = useRef();
  const inputRef_wifiPass = useRef();
  const inputRef_backendHost = useRef();
  const inputRef_backendPort = useRef();
  const inputRef_sampleDelay = useRef();

  const isConnectDisabled = isConnected || isConnecting;
  const isUploadDisabled = !isConnected || isUploadingConfig;

  const onConnect = useCallback(
    e => {
      e.preventDefault();

      if (!isConnectDisabled) {
        connect();
      }
    },
    [isConnectDisabled, connect]
  );

  const onUpload = useCallback(
    e => {
      e.preventDefault();

      if (isUploadDisabled) {
        return;
      }

      const form = formRef.current;
      if (!form.reportValidity()) {
        // form has invalid inputs
        return;
      }

      const wifiSSID = inputRef_wifiSSID.current.value;
      const wifiPass = inputRef_wifiPass.current.value;
      const backendHost = inputRef_backendHost.current.value;
      const backendPort = inputRef_backendPort.current.value;
      const sampleDelay = inputRef_sampleDelay.current.value;

      uploadConfig(wifiSSID, wifiPass, backendHost, backendPort, sampleDelay);
    },
    [isUploadDisabled, uploadConfig]
  );

  useEffect(() => {
    return () => {
      // before unmount
      if (isConnected) {
        disconnect();
      }
    };
  }, [isConnected, disconnect]);

  return (
    <form className={classes.root} autoComplete="off" ref={formRef}>
      <div className={classes.field}>
        <div
          className={cn(classes.connect, {
            [classes.disabled]: isConnectDisabled
          })}
          title={isConnected ? CONNECT_BUTTON_TITLE_DONE : CONNECT_BUTTON_TITLE}
        >
          <ConnectIcon width={50} onClick={onConnect} />
        </div>
      </div>

      <div className={classes.field}>
        <label htmlFor="wifi-ssid" className={classes.title}>
          Wi-Fi SSID
        </label>
        <input
          id="wifi-ssid"
          className={classes.value}
          required
          defaultValue={config.wifiSSID}
          disabled={!isConnected}
          ref={inputRef_wifiSSID}
        />
      </div>
      <div className={classes.field}>
        <label htmlFor="wifi-pass" className={classes.title}>
          Wi-Fi Password
        </label>
        <input
          id="wifi-pass"
          className={classes.value}
          type="password"
          defaultValue={config.wifiPass}
          disabled={!isConnected}
          ref={inputRef_wifiPass}
        />
      </div>
      <div className={classes.field}>
        <label htmlFor="backend-host" className={classes.title}>
          Backend Host
        </label>
        <input
          id="backend-host"
          className={classes.value}
          required
          defaultValue={config.backendHost}
          disabled={!isConnected}
          ref={inputRef_backendHost}
        />
      </div>
      <div className={classes.field}>
        <label htmlFor="backend-port" className={classes.title}>
          Backend Port
        </label>
        <input
          id="backend-port"
          className={classes.value}
          type="number"
          required
          min="0"
          max="65535"
          defaultValue={config.backendPort}
          disabled={!isConnected}
          ref={inputRef_backendPort}
        />
      </div>
      <div className={classes.field}>
        <label htmlFor="sample-delay" className={classes.title}>
          Sample Delay (sec)
        </label>
        <input
          id="sample-delay"
          className={classes.value}
          type="number"
          min="10"
          required
          defaultValue={config.sampleDelay}
          disabled={!isConnected}
          ref={inputRef_sampleDelay}
        />
      </div>

      <div className={classes.field}>
        <div
          title={UPLOAD_BUTTON_TITLE}
          className={cn(classes.upload, {
            [classes.disabled]: isUploadDisabled
          })}
        >
          <UploadIcon width={50} onClick={onUpload} />
        </div>
      </div>
    </form>
  );
}
