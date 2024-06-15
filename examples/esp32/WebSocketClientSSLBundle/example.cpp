/*
 * WebSocketClient.ino
 *
 *  Created on: 15.06.2024
 *
 */

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>

#include <WebSocketsClient.h>

extern const uint8_t rootca_crt_bundle_start[] asm(
    "_binary_data_cert_x509_crt_bundle_bin_start");

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

#define USE_SERIAL Serial1

void hexdump(const void * mem, uint32_t len, uint8_t cols = 16) {
    const uint8_t * src = (const uint8_t *)mem;
    USE_SERIAL.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
    for(uint32_t i = 0; i < len; i++) {
        if(i % cols == 0) {
            USE_SERIAL.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
        }
        USE_SERIAL.printf("%02X ", *src);
        src++;
    }
    USE_SERIAL.printf("\n");
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[WSc] Disconnected!\n");
            break;
        case WStype_CONNECTED:
            USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);

            // send message to server when Connected
            webSocket.sendTXT("Connected");
            break;
        case WStype_TEXT:
            USE_SERIAL.printf("[WSc] get text: %s\n", payload);

            // send message to server
            // webSocket.sendTXT("message here");
            break;
        case WStype_BIN:
            USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
            hexdump(payload, length);

            // send data to server
            // webSocket.sendBIN(payload, length);
            break;
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            break;
    }
}

void setup() {
    USE_SERIAL.begin(115200);

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

    // WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    // server address, port and URL (https://piehost.com/websocket-tester)
    webSocket.beginSslWithBundle(
        "demo.piesocket.com", 443, "/v3/channel_123?api_key=XYZ", rootca_crt_bundle_start);

    // event handler
    webSocket.onEvent(webSocketEvent);

    // use HTTP Basic Authorization this is optional remove if not needed
    webSocket.setAuthorization("user", "Password");

    // try ever 5000 again if connection has failed
    webSocket.setReconnectInterval(5000);
}

void loop() {
    webSocket.loop();
}
