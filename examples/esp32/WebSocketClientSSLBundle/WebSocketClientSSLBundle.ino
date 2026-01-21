/*
 * main.cpp
 *
 *  Created on: 15.06.2024
 *
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include <WebSocketsClient.h>

// Use the incbin library to embedd the cert binary
// extern const uint8_t rootca_crt_bundle_start[] asm(
//     "_binary_data_cert_x509_crt_bundle_bin_start");

WiFiMulti wifiMulti;
WebSocketsClient webSocket;

#define USE_SERIAL Serial

void setClock() {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    USE_SERIAL.print(F("Waiting for NTP time sync: "));
    time_t nowSecs = time(nullptr);
    while(nowSecs < 8 * 3600 * 2) {
        delay(500);
        USE_SERIAL.print(F("."));
        yield();
        nowSecs = time(nullptr);
    }

    USE_SERIAL.println();
    struct tm timeinfo;
    gmtime_r(&nowSecs, &timeinfo);
    USE_SERIAL.print(F("Current time: "));
    USE_SERIAL.print(asctime(&timeinfo));
}

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

    wifiMulti.addAP("SSID", "WIFI_PASSPHRASE");

    // WiFi.disconnect();
    while(wifiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    setClock();

    // server address, port and URL. This server can be flakey.
    // Expected response: Request served by 0123456789abcdef
    // webSocket.beginSslWithBundle("echo.websocket.org", 443, "/", rootca_crt_bundle_start, "");
    // ESP32 3.0.4 or higher needs the size of the bundle
    // webSocket.beginSslWithBundle("echo.websocket.org", 443, "/", rootca_crt_bundle_start, sizeof(rootca_crt_bundle_start), "");
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 4)
    webSocket.beginSslWithBundle("echo.websocket.org", 443, "/", NULL, 0, "");
#else
    webSocket.beginSslWithBundle("echo.websocket.org", 443, "/", NULL, "");
#endif

    // event handler
    webSocket.onEvent(webSocketEvent);

    // use HTTP Basic Authorization this is optional enable if needed
    // webSocket.setAuthorization("user", "Password");

    // try ever 5000 again if connection has failed
    webSocket.setReconnectInterval(5000);
}

void loop() {
    webSocket.loop();
}
