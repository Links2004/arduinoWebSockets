/*
 * WebSocketClient.ino
 *
 *  Created on: 27.05.2018
 *
 */
#include <Ethernet2.h>
#include <WebSocketsClient.h>
#include <Streaming.h>

WebSocketsClient webSocket;

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF};
IPAddress ip(192, 168, 0, 110);
#define USE_SERIAL Serial

void webSocketEvent(WStype_t type, uint8_t * payload, size_t lenght) {


    switch(type) {
        case WStype_DISCONNECTED:
            //USE_SERIAL.printf("[WSc] Disconnected!\n");
            USE_SERIAL << "[WSc] Disconnected!" << endl;
            break;
        case WStype_CONNECTED:
            {
                //USE_SERIAL.printf("[WSc] Connected to url: %s\n",  payload);
				        USE_SERIAL << "[WSc] Connected to url: " << (char*)payload << endl;
			    // send message to server when Connected
				webSocket.sendTXT("Connected");
            }
            break;
        case WStype_TEXT:
            //USE_SERIAL.printf("[WSc] get text: %s\n", payload);
            USE_SERIAL << "[WSc] get text: " << (char*)payload << endl;

			// send message to server
			// webSocket.sendTXT("message here");
            break;
        case WStype_BIN:
            //USE_SERIAL.printf("[WSc] get binary lenght: %u\n", lenght);
            USE_SERIAL << "[WSc] get binary lenght: " << lenght << endl;
            //hexdump(payload, lenght);

            // send data to server
            // webSocket.sendBIN(payload, lenght);
            break;
    }

}

void setup() {
    // USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

      for(uint8_t t = 4; t > 0; t--) {
          //USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
          USE_SERIAL << "[SETUP] BOOT WAIT " << t << "..." << endl;
          USE_SERIAL.flush();
          delay(1000);
      }

   Ethernet.begin(mac,ip);
   USE_SERIAL << Ethernet.localIP() << endl;



    //webSocket.begin("wss://echo.websocket.org", 443);
    webSocket.begin("ws://echo.websocket.org", 80);
    webSocket.onEvent(webSocketEvent);

}

void loop() {
    webSocket.loop();
}