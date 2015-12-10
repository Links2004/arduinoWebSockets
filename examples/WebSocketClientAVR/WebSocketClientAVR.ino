/*
 * WebSocketClientAVR.ino
 *
 *  Created on: 10.12.2015
 *
 */

#include <Arduino.h>

#include <SPI.h>
#include <Ethernet.h>

#include <WebSocketsClient.h>



// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);

WebSocketsClient webSocket;



void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {


    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("[WSc] Disconnected!\n");
            break;
        case WStype_CONNECTED:
            {
                Serial.print("[WSc] Connected to url: ");
                Serial.println((char *)payload);
                // send message to server when Connected
                webSocket.sendTXT("Connected");
            }
            break;
        case WStype_TEXT:
            Serial.print("[WSc] get text: ");
            Serial.println((char *)payload);
            // send message to server
            // webSocket.sendTXT("message here");
            break;
        case WStype_BIN:
            Serial.print("[WSc] get binary length: ");
            Serial.println(length);
           // hexdump(payload, length);

            // send data to server
            // webSocket.sendBIN(payload, length);
            break;
    }

}

void setup()
{
    // Open serial communications and wait for port to open:
    Serial.begin(115200);
    while (!Serial) {}

    // start the Ethernet connection:
    if (Ethernet.begin(mac) == 0) {
      Serial.println("Failed to configure Ethernet using DHCP");
      // no point in carrying on, so do nothing forevermore:
      // try to congifure using IP address instead of DHCP:
      Ethernet.begin(mac, ip);
    }

    webSocket.begin("192.168.0.123", 8011);
    webSocket.onEvent(webSocketEvent);

}


void loop()
{
    webSocket.loop();
}
