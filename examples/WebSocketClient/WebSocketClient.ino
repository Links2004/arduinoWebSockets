/*
 * WebSocketClient.ino
 *
 *  Created on: 24.05.2015
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <WebSocketsClient.h>

#include <Hash.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;


void webSocketEvent(WStype_t type, uint8_t * payload, size_t lenght) {


    switch(type) {
        case WStype_DISCONNECTED:
            Serial1.printf("[WSc] Disconnected!\n");
            break;
        case WStype_CONNECTED:
            {
                Serial1.printf("[WSc] Connected to url: %s\n",  payload);
            }
            break;
        case WStype_TEXT:
            Serial1.printf("[WSc] get text: %s\n", lenght);

            // send data to back to Server
            webSocket.sendTXT(payload, lenght);
            break;
        case WStype_BIN:
            Serial1.printf("[WSc] get binary lenght: %u\n", lenght);
            hexdump(payload, lenght);

            // echo data back to Server
            webSocket.sendBIN(payload, lenght);
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

    //WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    webSocket.begin("192.168.0.123", 81);
    webSocket.onEvent(webSocketEvent);

}



void loop() {
    webSocket.loop();
}
