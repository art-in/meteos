import React, {useRef, useCallback, useEffect} from 'react';

import useConfigurator from 'hooks/use-configurator';
import classes from './ConfigForm.css';

const CONNECT_BUTTON_TITLE =
  'Connects to sensor over bluetooth (make sure device is in config mode)';
const UPDATE_BUTTON_TITLE = 'Sends configuration to sensor';

export default function ConfigForm() {
  const {
    isConnected,
    connect,
    disconnect,
    config,
    updateConfig,
    isUpdatingConfig
  } = useConfigurator();

  const formRef = useRef();
  const inputRefWifiSSID = useRef();
  const inputRefWifiPass = useRef();
  const inputRefBackendHost = useRef();
  const inputRefBackendPort = useRef();
  const inputRefSampleDelay = useRef();

  const onConnect = useCallback(
    e => {
      e.preventDefault();
      connect();
    },
    [connect]
  );

  const onUpdate = useCallback(
    e => {
      e.preventDefault();

      const form = formRef.current;
      if (!form.reportValidity()) {
        // form has invalid inputs
        return;
      }

      const wifiSSID = inputRefWifiSSID.current.value;
      const wifiPass = inputRefWifiPass.current.value;
      const backendHost = inputRefBackendHost.current.value;
      const backendPort = inputRefBackendPort.current.value;
      const sampleDelay = inputRefSampleDelay.current.value;

      updateConfig(wifiSSID, wifiPass, backendHost, backendPort, sampleDelay);
    },
    [updateConfig]
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
        <button
          title={CONNECT_BUTTON_TITLE}
          disabled={isConnected}
          onClick={onConnect}
        >
          Connect
        </button>
      </div>

      <div className={classes.field}>
        <label htmlFor="wifi-ssid" className={classes.title}>
          WiFi SSID
        </label>
        <input
          id="wifi-ssid"
          className={classes.value}
          required
          defaultValue={config.wifiSSID}
          disabled={!isConnected}
          ref={inputRefWifiSSID}
        />
      </div>
      <div className={classes.field}>
        <label htmlFor="wifi-pass" className={classes.title}>
          WiFi Password
        </label>
        <input
          id="wifi-pass"
          className={classes.value}
          type="password"
          defaultValue={config.wifiPass}
          disabled={!isConnected}
          ref={inputRefWifiPass}
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
          ref={inputRefBackendHost}
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
          ref={inputRefBackendPort}
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
          ref={inputRefSampleDelay}
        />
      </div>

      <div className={classes.field}>
        <button
          title={UPDATE_BUTTON_TITLE}
          disabled={!isConnected || isUpdatingConfig}
          onClick={onUpdate}
        >
          Update
        </button>
      </div>
    </form>
  );
}
