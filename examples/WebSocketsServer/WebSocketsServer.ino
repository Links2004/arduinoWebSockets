/*
 * WebSocketsServer.ino
 *
 *  Created on: 22.05.2015
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>

ESP8266WiFiMulti WiFiMulti;

WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial1.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial1.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            }
            break;
        case WStype_TEXT:
            Serial1.printf("[%u] get Text: %s\n", num, payload);

            // echo data back to browser
            webSocket.sendTXT(num, payload, lenght);

            // send data to all connected clients
            webSocket.broadcastTXT(payload, lenght);
            break;
        case WStype_BIN:
            Serial1.printf("[%u] get binary lenght: %u\n", num, lenght);
            hexdump(payload, lenght);

            // echo data back to browser
            webSocket.sendBIN(num, payload, lenght);
            break;
    }

}

void setup() {
    Serial.begin(921600);
    Serial1.begin(921600);

    //Serial.setDebugOutput(true);
    Serial1.setDebugOutput(true);

    Serial1.println();
    Serial1.println();
    Serial1.println();

    for(uint8_t t = 4; t > 0; t--) {
        Serial1.printf("[SETUP] BOOT WAIT %d...\n", t);
        Serial1.flush();
        delay(1000);
    }

    WiFiMulti.addAP("SSID", "passpasspass");

    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    webSocket.loop();
}

