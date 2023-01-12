/*
    WebSocketClientStomp.ino

    Example for connecting and maintining a connection with a STOMP websocket connection.
    In this example, we connect to a Spring application (see https://docs.spring.io/spring/docs/current/spring-framework-reference/html/websocket.html).

    Created on: 25.09.2017
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

const char* wlan_ssid             = "yourssid";
const char* wlan_password         = "somepassword";

const char* ws_host               = "the.host.net";
const int   ws_port               = 80;

// URL for STOMP endpoint.
// For the default config of Spring's STOMP support, the default URL is "/socketentry/websocket".
const char* stompUrl            = "/socketentry/websocket"; // don't forget the leading "/" !!!


// VARIABLES

WebSocketsClient webSocket;


// FUNCTIONS

/**
 * STOMP messages need to be NULL-terminated (i.e., \0 or \u0000).
 * However, when we send a String or a char[] array without specifying 
 * a length, the size of the message payload is derived by strlen() internally,
 * thus dropping any NULL values appended to the "msg"-String.
 * 
 * To solve this, we first convert the String to a NULL terminated char[] array
 * via "c_str" and set the length of the payload to include the NULL value.
 */
void sendMessage(String & msg) {
    webSocket.sendTXT(msg.c_str(), msg.length() + 1);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

    switch (type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[WSc] Disconnected!\n");
            break;
        case WStype_CONNECTED:
            {
                USE_SERIAL.printf("[WSc] Connected to url: %s\n",  payload);
                
                String msg = "CONNECT\r\naccept-version:1.1,1.0\r\nheart-beat:10000,10000\r\n\r\n";
                sendMessage(msg);
            }
            break;
        case WStype_TEXT:
            {
                // #####################
                // handle STOMP protocol
                // #####################

                String text = (char*) payload;
                USE_SERIAL.printf("[WSc] get text: %s\n", payload);

                if (text.startsWith("CONNECTED")) {

                    // subscribe to some channels

                    String msg = "SUBSCRIBE\nid:sub-0\ndestination:/user/queue/messages\n\n";
                    sendMessage(msg);
                    delay(1000);

                    // and send a message

                    msg = "SEND\ndestination:/app/message\n\n{\"user\":\"esp\",\"message\":\"Hello!\"}";
                    sendMessage(msg);
                    delay(1000);
                    
                } else {

                    // do something with messages
                    
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


    // connect to websocket
    webSocket.begin(ws_host, ws_port, stompUrl);
    webSocket.setExtraHeaders(); // remove "Origin: file://" header because it breaks the connection with Spring's default websocket config
    //    webSocket.setExtraHeaders("foo: I am so funny\r\nbar: not"); // some headers, in case you feel funny
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    webSocket.loop();
}
