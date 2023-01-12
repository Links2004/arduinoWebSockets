/*
 * WebSocketServer.ino
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

#define USE_SERIAL Serial

String fragmentBuffer = "";

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

	switch(type) {
		case WStype_DISCONNECTED:
			USE_SERIAL.printf("[%u] Disconnected!\n", num);
			break;
		case WStype_CONNECTED: {
			IPAddress ip = webSocket.remoteIP(num);
			USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

			// send message to client
			webSocket.sendTXT(num, "Connected");
		}
			break;
		case WStype_TEXT:
			USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
			break;
		case WStype_BIN:
			USE_SERIAL.printf("[%u] get binary length: %u\n", num, length);
			hexdump(payload, length);
			break;

		// Fragmentation / continuation opcode handling
		// case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT_TEXT_START:
			fragmentBuffer = (char*)payload;
			USE_SERIAL.printf("[%u] get start start of Textfragment: %s\n", num, payload);
			break;
		case WStype_FRAGMENT:
			fragmentBuffer += (char*)payload;
			USE_SERIAL.printf("[%u] get Textfragment : %s\n", num, payload);
			break;
		case WStype_FRAGMENT_FIN:
			fragmentBuffer += (char*)payload;
			USE_SERIAL.printf("[%u] get end of Textfragment: %s\n", num, payload);
			USE_SERIAL.printf("[%u] full frame: %s\n", num, fragmentBuffer.c_str());
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

	webSocket.begin();
	webSocket.onEvent(webSocketEvent);
}

void loop() {
	webSocket.loop();
}

