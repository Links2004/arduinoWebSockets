/*
 * WebSocketClientOTA.ino
 *
 *  Created on: 25.10.2021
 *
 */

#include <Arduino.h>
#include <ArduinoJson.h>

#ifdef ESP8266
    #include <ESP8266WiFi.h>
    #include <ESP8266mDNS.h>
	#include <Updater.h>
#endif
#ifdef ESP32
    #include "WiFi.h"
    #include "ESPmDNS.h"
	#include <Update.h>
#endif

#include <WiFiUdp.h>
#include <ESP8266WiFiMulti.h>

#include <WebSocketsClient.h>

#include <Hash.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

#define USE_SERIAL Serial

// Variables:
// Settable:
const char *version = "1.0.0";
const char *name = "mydevice";

// Others:
#ifdef ESP8266
	const char *chip = "esp8266";
#endif
#ifdef ESP32
	const char *chip = "esp32";
#endif

uint32_t maxSketchSpace = 0;
int SketchSize = 0;
bool ws_conn = false;

String IpAddress2String(const IPAddress& ipAddress)
{
    return String(ipAddress[0]) + String(".") +
           String(ipAddress[1]) + String(".") +
           String(ipAddress[2]) + String(".") +
           String(ipAddress[3]);
}

void greetings_(){
	StaticJsonDocument<200> doc;
	doc["type"] = "greetings";
	doc["mac"] = WiFi.macAddress();
	doc["ip"] = IpAddress2String(WiFi.localIP());
	doc["version"] = version;
	doc["name"] = name;
	doc["chip"] = chip;

	char data[200];
	serializeJson(doc, data);
	webSocket.sendTXT(data);
}

void register_(){
	StaticJsonDocument<200> doc;
	doc["type"] = "register";
    doc["mac"] = WiFi.macAddress();

	char data[200];
	serializeJson(doc, data);
	webSocket.sendTXT(data);
    ws_conn = true;
}

typedef void (*CALLBACK_FUNCTION)(JsonDocument &msg);

typedef struct {
   char  type[50];
   CALLBACK_FUNCTION  func;
} RESPONSES_STRUCT;

void OTA(JsonDocument &msg){
    USE_SERIAL.print(F("[WSc] OTA mode: "));
    const char* go = "go";
    const char* ok = "ok";
    if(strncmp( msg["value"], go, strlen(go)) == 0 ) {
        USE_SERIAL.print(F("go\n"));
        SketchSize = int(msg["size"]);
        maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        USE_SERIAL.printf("[WSc] Max sketch size: %u\n", maxSketchSpace);
        USE_SERIAL.printf("[WSc] Sketch size: %d\n", SketchSize);
        USE_SERIAL.setDebugOutput(true);
        if (!Update.begin(maxSketchSpace)) { //start with max available size
            Update.printError(Serial);
            ESP.restart();
        }
    } else if (strncmp( msg["value"], ok, strlen(ok)) == 0) {
        USE_SERIAL.print(F("OK\n"));
        register_();
    } else {
        USE_SERIAL.print(F("unknown value : "));
        USE_SERIAL.print(msg["value"].as<char>());
        USE_SERIAL.print(F("\n"));
    }
}

void STATE(JsonDocument &msg){
    // Do something with message
}

RESPONSES_STRUCT responses[] = {
  {"ota",           OTA},
  {"state",      STATE},
};

void text(uint8_t * payload, size_t length){
    // Convert mesage to something usable
    char msgch[length];
    for (unsigned int i = 0; i < length; i++)
    {
        USE_SERIAL.print((char)payload[i]);
        msgch[i] = ((char)payload[i]);
    }
    msgch[length] = '\0';

    // Parse Json
    StaticJsonDocument<200> doc_in;
    DeserializationError error = deserializeJson(doc_in, msgch);

    if (error) {
        USE_SERIAL.print(F("deserializeJson() failed: "));
        USE_SERIAL.println(error.c_str());
        return;
    }

    // Handle each TYPE of message
    int b = 0;

    for( b=0 ; strlen(responses[b].type) ; b++ )
    {
        if( strncmp(doc_in["type"], responses[b].type, strlen(responses[b].type)) == 0 ) {
            responses[b].func(doc_in);
        }
    }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

	switch(type) {
		case WStype_DISCONNECTED:
			USE_SERIAL.printf("[WSc] Disconnected!\n");
			break;
		case WStype_CONNECTED: {
			USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);

			// send message to server when Connected
			// webSocket.sendTXT("Connected");
			greetings_();
		}
			break;
		case WStype_TEXT:
			USE_SERIAL.printf("[WSc] get text: %s\n", payload);

			// send message to server
			// webSocket.sendTXT("message here");
			text(payload, length);
			break;
		case WStype_BIN:
			USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
			// hexdump(payload, length);
	        if (Update.write(payload, length) != length) {
                Update.printError(Serial);
                ESP.restart();
            }
            yield();
            SketchSize -= length;
            USE_SERIAL.printf("[WSc] Sketch size left: %u\n", SketchSize);
            if (SketchSize < 1){
                if (Update.end(true)) { //true to set the size to the current progress
                    USE_SERIAL.printf("Update Success: \nRebooting...\n");
                    delay(5);
                    yield();
                    ESP.restart();
                } else {
                    Update.printError(USE_SERIAL);
                    ESP.restart();
                }
                USE_SERIAL.setDebugOutput(false);
            }

			// send data to server
			// webSocket.sendBIN(payload, length);
			break;
        case WStype_PING:
            // pong will be send automatically
            USE_SERIAL.printf("[WSc] get ping\n");
            break;
        case WStype_PONG:
            // answer to a ping we send
            USE_SERIAL.printf("[WSc] get pong\n");
            break;
    }

}

void setup() {
	// USE_SERIAL.begin(921600);
	USE_SERIAL.begin(115200);

	//Serial.setDebugOutput(true);
	USE_SERIAL.setDebugOutput(true);

	USE_SERIAL.print(F("\nMAC: "));
	USE_SERIAL.println(WiFi.macAddress());
	USE_SERIAL.print(F("\nDevice: "));
	USE_SERIAL.println(name);
	USE_SERIAL.printf("\nVersion: %s\n", version);

	for(uint8_t t = 4; t > 0; t--) {
		USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
		USE_SERIAL.flush();
		delay(1000);
	}

	WiFiMulti.addAP("SSID", "PASS");

	//WiFi.disconnect();
	while(WiFiMulti.run() != WL_CONNECTED) {
		delay(100);
	}

	// server address, port and URL
	webSocket.begin("10.0.1.5", 8081, "/");

	// event handler
	webSocket.onEvent(webSocketEvent);

	// use HTTP Basic Authorization this is optional remove if not needed
	// webSocket.setAuthorization("USER", "PASS");

	// try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(5000);

  // start heartbeat (optional)
  // ping server every 15000 ms
  // expect pong from server within 3000 ms
  // consider connection disconnected if pong is not received 2 times
  webSocket.enableHeartbeat(15000, 3000, 2);

}

void loop() {
	webSocket.loop();
}
