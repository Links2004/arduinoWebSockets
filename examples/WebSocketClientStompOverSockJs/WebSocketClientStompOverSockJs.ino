/*
    WebSocketClientStompOverSockJs.ino

    Example for connecting and maintining a connection with a SockJS+STOMP websocket connection.
    In this example, we connect to a Spring application (see https://docs.spring.io/spring/docs/current/spring-framework-reference/html/websocket.html).
    In particular, we use the default STOMP WebSocket example by Spring: https://github.com/spring-guides/gs-messaging-stomp-websocket
    That is, we connect to the Spring server, send a "hello" and get back a "greeting".

    Created on: 18.07.2017
    Author: Martin Becker <mgbckr>, Contact: becker@informatik.uni-wuerzburg.de
*/

// PRE

#define USE_SERIAL Serial


// LIBRARIES

#include <Arduino.h>
#include <Hash.h>

#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>


// SETTINGS

const char* wlan_ssid             = "<ssid>";
const char* wlan_password         = "<password>";

const char* ws_host               = "<ip>";
const int   ws_port               = 8080; // the Spring webapp runs on port 8080 by default (instead of 80 for standard HTTP)

// base URL for SockJS (websocket) connection
// The complete URL will look something like this (cf. http://sockjs.github.io/sockjs-protocol/sockjs-protocol-0.3.3.html#section-36):
//
//     ws://<ws_host>:<ws_port>/<ws_baseurl>/<3digits>/<randomstring>/websocket
//
// For the default config of Spring's SockJS/STOMP support, the preset base URL is "/socketentry/".
// Here, we use the URL defined by the Spring example mentioned in the preamble.
const char* ws_baseurl            = "/gs-guide-websocket/"; // don't forget leading and trailing "/" !!!


// VARIABLES

WebSocketsClient webSocket;


// FUNCTIONS

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

    switch (type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[WSc] Disconnected!\n");
            break;
        case WStype_CONNECTED:
            {
                USE_SERIAL.printf("[WSc] Connected to url: %s\n",  payload);
            }
            break;
        case WStype_TEXT:
            {
                // #####################
                // handle SockJs+STOMP protocol
                // #####################

                String text = (char*) payload;

                USE_SERIAL.printf("[WSc] get text: %s\n", payload);

                if (payload[0] == 'h') {

                    USE_SERIAL.println("Heartbeat!");

                } else if (payload[0] == 'o') {

                    // on open connection
                    char *msg = "[\"CONNECT\\naccept-version:1.1,1.0\\nheart-beat:10000,10000\\n\\n\\u0000\"]";
                    webSocket.sendTXT(msg);

                } else if (text.startsWith("a[\"CONNECTED")) {

                    // subscribe to some channels

                    char *msg = "[\"SUBSCRIBE\\nid:sub-0\\ndestination:/topic/greetings\\n\\n\\u0000\"]";
                    webSocket.sendTXT(msg);
                    delay(1000);

                    // and send a message

                    msg = "[\"SEND\\ndestination:/app/hello\\n\\n{\\\"name\\\":\\\"blubb\\\"}\\u0000\"]";
                    webSocket.sendTXT(msg);
                    delay(1000);
                }

                break;
            }
        case WStype_BIN:
            USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
            hexdump(payload, length);

            // send data to server
            // webSocket.sendBIN(payload, length);
            break;
    }

}

void setup() {

    // setup serial

    // USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);

    //    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();


    // connect to WiFi

    USE_SERIAL.print("Logging into WLAN: "); Serial.print(wlan_ssid); Serial.print(" ...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(wlan_ssid, wlan_password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        USE_SERIAL.print(".");
    }
    USE_SERIAL.println(" success.");
    USE_SERIAL.print("IP: "); USE_SERIAL.println(WiFi.localIP());


    // #####################
    // create socket url according to SockJS protocol (cf. http://sockjs.github.io/sockjs-protocol/sockjs-protocol-0.3.3.html#section-36)
    // #####################
    String socketUrl = ws_baseurl;
    socketUrl += random(0, 999);
    socketUrl += "/";
    socketUrl += random(0, 999999); // should be a random string, but this works (see )
    socketUrl += "/websocket";

    // connect to websocket
    webSocket.begin(ws_host, ws_port, socketUrl);
    webSocket.setExtraHeaders(); // remove "Origin: file://" header because it breaks the connection with Spring's default websocket config
    //    webSocket.setExtraHeaders("foo: I am so funny\r\nbar: not"); // some headers, in case you feel funny
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    webSocket.loop();
}
