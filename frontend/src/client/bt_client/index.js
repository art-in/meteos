const SERVICE_UUID = "b8acbbea-21c2-4c6a-a52f-6033f2f6d1a6";
const SSID_CHARACTERISTIC_UUID = "d1fe36b7-3223-4927-ada5-422f2b5afcf9";
const PASS_CHARACTERISTIC_UUID = "5e527e75-2d5a-40f1-95fb-0e4a7d612ee9";

let service;
let ssidCharacteristic;
let passCharacteristic;

async function onConnectButtonClick() {
    console.log('Requesting Bluetooth Device...');
    const device = await navigator.bluetooth.requestDevice({ filters: [{ services: [SERVICE_UUID] }] });

    console.log('Connecting to GATT Server...');
    const server = await device.gatt.connect();

    console.log('Getting Service...');
    service = await server.getPrimaryService(SERVICE_UUID);

    console.log('Connected to service.');

    console.log('Getting Characteristics...');
    var dec = new TextDecoder(); // utf-8
    let valueBuffer;

    ssidCharacteristic = await service.getCharacteristic(SSID_CHARACTERISTIC_UUID);
    valueBuffer = await ssidCharacteristic.readValue();
    const ssid = dec.decode(valueBuffer);
    document.querySelector('#ssid').value = ssid;

    passCharacteristic = await service.getCharacteristic(PASS_CHARACTERISTIC_UUID);
    valueBuffer = await passCharacteristic.readValue();
    const pass = dec.decode(valueBuffer);
    document.querySelector('#pass').value = pass;

    console.log('Values read');
}

async function onUpdateButtonClick() {
    if (!service) {
        throw Error('Not connected to service yet.');
    }

    const ssid = document.querySelector('input#ssid').value;
    const pass = document.querySelector('input#pass').value;
    if (!ssid || !pass) {
        throw Error('No SSID or pass specified.')
    }

    console.log('Writing SSID/Pass values...');
    var enc = new TextEncoder(); // utf-8

    await ssidCharacteristic.writeValue(enc.encode(ssid));
    await passCharacteristic.writeValue(enc.encode(pass));

    console.log('Values written.')
}

document.querySelector('button#connect').addEventListener('click', onConnectButtonClick);
document.querySelector('button#update').addEventListener('click', onUpdateButtonClick);
