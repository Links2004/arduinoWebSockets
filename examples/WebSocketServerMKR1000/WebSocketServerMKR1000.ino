/*
 * WebSocketServer.ino
 *
 *  Created on: 22.05.2015
 *
 */

#include <Arduino.h>

#include <SPI.h>
#include <WiFi101.h>
#include <WebSocketsServer.h>


char ssid[] = "";      //  your network SSID (name)
char pass[] = "";   // your network password
int keyIndex = 0; 

WebSocketsServer webSocket = WebSocketsServer(8003);

#define USE_SERIAL Serial

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
//            USE_SERIAL.print(num);
           // USE_SERIAL.println(" Disconnected!\n");
            break;
        case WStype_CONNECTED:
            {
                //IPAddress ip = webSocket.remoteIP(num);
                //USE_SERIAL.println("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
				        USE_SERIAL.println("connected");
				// send message to client
				webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            USE_SERIAL.print((char*)payload);
            USE_SERIAL.println(" received");

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            USE_SERIAL.println("got some binary");
//            USE_SERIAL.println("[%u] get binary lenght: %u\n", num, lenght);
//            hexdump(payload, lenght);

            // send message to client
            // webSocket.sendBIN(num, payload, lenght);
            break;
    }

}

void setup() {
    // USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);

    //Serial.setDebugOutput(true);
//    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
      USE_SERIAL.print(t);
        USE_SERIAL.println(" ... [SETUP] BOOT WAIT ...\n");
        USE_SERIAL.flush();
        delay(1000);
    }
    ///CONNECT TO WIFI
    if (WiFi.status() == WL_NO_SHIELD) {
      Serial.println("NOT PRESENT");
      return; // don't continue
    }
    Serial.println("WIFI MODULE DETECTED");
    // attempt to connect to Wifi network:
    while ( WiFi.status() != WL_CONNECTED) {
      
      Serial.print("Attempting to connect to Network named: ");
      Serial.println(ssid);                   // print the network name (SSID);
      
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
       WiFi.begin(ssid, pass);
      // wait 10 seconds for connection:
      delay(10000);
    }
    Serial.print("Connected to ");
    Serial.print(ssid);
    Serial.print(" with IP:");
    IPAddress ip = WiFi.localIP();
    Serial.println(ip);
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    webSocket.loop();
    delay(200);
}

