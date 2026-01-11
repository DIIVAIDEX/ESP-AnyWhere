// var gateway = `wss://disp.vmk-a.com/esp32s3/ws`;
// var gateway = `ws://172.16.4.8/ws`;
// var gateway = `wss://${window.location.host}${window.location.pathname}/ws`;
var gateway = `ws://${window.location.host}/ws`;
var ws;
window.addEventListener('load', onload);

const saveBtnWg = document.getElementById('saveWgBtn');
const saveBtnSoftAP = document.getElementById('saveSoftAPBtn');
const saveBtnWifi = document.getElementById('saveWifiBtn');
const statusElement = document.getElementById('connectionStatus');

function onload(event){
    initWebSocket();
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection');
    ws = new WebSocket(gateway);
    ws.onopen = onOpen;
    ws.onclose = onClose;
    ws.onmessage = onMessage;
}

// When websocket is established, call the getReadings() function
function onOpen(event) {
    console.log('Connection opened');
    statusElement.textContent = 'Connected to ESP32';
    eventSend('getReadings');
}

function onClose(event) {
    console.log('Connection closed');
    statusElement.textContent = 'Cannot connect to ESP32';
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    console.log(event.data);
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    if(myObj.type === 'info'){
        updateDeviceInfo(myObj);
    }
    else if(myObj.type === 'settings'){
        for (var i = 0; i < keys.length; i++){
            var key = keys[i];
            if(key !== 'type')document.getElementById(key).value = myObj[key];
        }
        if(saveBtnWg.textContent === 'Сохранение...'){
            SavedNotifBtn(saveBtnWg);
        }
        else if(saveBtnSoftAP.textContent === 'Сохранение...'){
            SavedNotifBtn(saveBtnSoftAP);
        }
        else if(saveBtnWifi.textContent === 'Сохранение...'){
            SavedNotifBtn(saveBtnWifi);
        }
    }
    else if(myObj.type === 'inputs'){
        for (var i = 0; i < keys.length; i++){
            var key = keys[i];
            if(key !== 'type')(document.getElementById(key)).checked = !myObj[key];
        }
    }
    else if(myObj.type === 'ioNaming'){
        for (var i = 0; i < keys.length; i++){
            var key = keys[i];
            if(key !== 'type')(document.getElementById(key)).textContent = myObj[key];
        }
    }
    else if(myObj.type === 'log'){
        const cleanedData = cleanAnsiCodes(myObj.log);
        appendToTextField(cleanedData);
    }
    else{
        for (var i = 0; i < keys.length; i++){
            var key = keys[i];
            document.getElementById(key).innerHTML = myObj[key];
        }
    }
}

function SendJsonParam(param, val) {
    var comJson = {
        [param]: val
    }
    ws.send(JSON.stringify(comJson));
    console.log(comJson);
}

function SendStringWS(stringToSend) {
    if (window.ws || ws.readyState == WebSocket.OPEN) {
        ws.send(stringToSend);
    }
}

function eventSend(element){
    SendStringWS(element);
}

function eventIoSend(element){
    SendJsonParam(element, (document.getElementById(element)).checked);
}

function SaveBtn(element){
    var currBtn;
    if(element === 'wireguard'){
        settingsData = {
            typeJson: element,
            wgPrivateKey: document.getElementById('wgPrivateKey').value,
            wgLocalIP: document.getElementById('wgLocalIP').value,
            wgLocalMask: document.getElementById('wgLocalMask').value,
            wgPublicPeerKey: document.getElementById('wgPublicPeerKey').value,
            wgPeerAddress: document.getElementById('wgPeerAddress').value,
            wgPeerPort: Number(document.getElementById('wgPeerPort').value)
        };
        currBtn = saveBtnWg;
    }
    else if(element === 'wifiAp'){
        settingsData = {
            typeJson: element,
            wifiApName: document.getElementById('wifiApName').value,
            wifiApPass: document.getElementById('wifiApPass').value,
            wifiApCh: Number(document.getElementById('wifiApCh').value)
        };
        currBtn = saveBtnSoftAP;
    }
    else if(element === 'wifiSta'){
        settingsData = {
            typeJson: element,
            wifiStaName: document.getElementById('wifiStaName').value,
            wifiStaPass: document.getElementById('wifiStaPass').value
        };
        currBtn = saveBtnWifi;
    }

    if (ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify(settingsData));
        console.log(settingsData);
        currBtn.textContent = 'Сохранение...';
        currBtn.style.backgroundColor = '#ffa500'; // Оранжевый цвет при сохранении
    }
    else{
        console.log('Cannot to send settings!');
    }
};

function updateDeviceInfo(data) {
    // Форматируем данные для отображения
    let html = `<br/>
        ${data.operator}, RSSI: ${data.rssi}<br/>
        Bat: ${data.vBat.toFixed(2)}V, ${data.vBatPerc.toFixed(0)}%<br/>
        CPU: ${data.cpuTemp.toFixed(0)}°C, FreeHeap: ${data.freeHeap}
    `;
    document.getElementById('deviceInfo').innerHTML = html;
    
    // if(data.netStatus) statusElement.textContent = 'Контроллер подключен к серверу';
    // else statusElement.textContent = 'Контроллер НЕ в сети!';
}

function cleanAnsiCodes(text) {
    return text.replace(/\u001b\[[0-9;]*m/g, '');
}

function appendToTextField(text){
    const textarea = document.getElementById('logTextarea');
    textarea.value += text; // Добавляем новую строку
    textarea.scrollTop = textarea.scrollHeight; // Автопрокрутка вниз
}

function SavedNotifBtn(currBtn){
    currBtn.textContent = 'Сохранено!';
    currBtn.style.backgroundColor = '#00D609';
    setTimeout(function(){
        currBtn.textContent = 'Спаси&Сохрани';
        currBtn.style.backgroundColor = '#FF0000'; 
    }, 2000);
}
