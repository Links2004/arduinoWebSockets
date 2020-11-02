/*
 * WebSocketServerHooked.ino
 *
 *  Created on: 22.05.2015
 *  Hooked on:  28.10.2020
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSockets4WebServer.h>
#include <Hash.h>
#include <ESP8266mDNS.h>

ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer server(80);
WebSockets4WebServer webSocket;

#define USE_SERIAL Serial

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

				// send message to client
				webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            USE_SERIAL.printf("[%u] get binary length: %u\n", num, length);
            hexdump(payload, length);

            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;
    }

}

void setup() {
    // USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);

    //Serial.setDebugOutput(true);
    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    WiFiMulti.addAP("SSID", "passpasspass");

    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    server.on("/", []() {
        server.send(200, "text/plain", "I am a regular webserver on port 80!\r\n");
        server.send(200, "text/plain", "I am also a websocket server on '/ws' on the same port 80\r\n");
    });

    server.addHook(webSocket.hookForWebserver("/ws", webSocketEvent));

    server.begin();
    Serial.println("HTTP server started on port 80");
    Serial.println("WebSocket server started on the same port");
    Serial.printf("my network address is either 'arduinoWebsockets.local' (mDNS) or '%s'\n", WiFi.localIP().toString().c_str());

    if (!MDNS.begin("arduinoWebsockets")) {
        Serial.println("Error setting up MDNS responder!");
    }
}

void loop() {
    server.handleClient();
    webSocket.loop();
    MDNS.update();
}
