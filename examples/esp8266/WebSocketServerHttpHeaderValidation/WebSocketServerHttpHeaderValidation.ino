/*
 * WebSocketServerHttpHeaderValidation.ino
 *
 *  Created on: 08.06.2016
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>

ESP8266WiFiMulti WiFiMulti;

WebSocketsServer webSocket = WebSocketsServer(81);

#define USE_SERIAL Serial1

const unsigned long int validSessionId = 12345; //some arbitrary value to act as a valid sessionId

/*
 * Returns a bool value as an indicator to describe whether a user is allowed to initiate a websocket upgrade
 * based on the value of a cookie. This function expects the rawCookieHeaderValue to look like this "sessionId=<someSessionIdNumberValue>|"
 */
bool isCookieValid(String rawCookieHeaderValue) {

	if (rawCookieHeaderValue.indexOf("sessionId") != -1) {
		String sessionIdStr = rawCookieHeaderValue.substring(rawCookieHeaderValue.indexOf("sessionId=") + 10, rawCookieHeaderValue.indexOf("|"));
		unsigned long int sessionId = strtoul(sessionIdStr.c_str(), NULL, 10);
		return sessionId == validSessionId;
	}
	return false;
}

/*
 * The WebSocketServerHttpHeaderValFunc delegate passed to webSocket.onValidateHttpHeader
 */
bool validateHttpHeader(String headerName, String headerValue) {

	//assume a true response for any headers not handled by this validator
	bool valid = true;

	if(headerName.equalsIgnoreCase("Cookie")) {
		//if the header passed is the Cookie header, validate it according to the rules in 'isCookieValid' function
		valid = isCookieValid(headerValue);
	}

	return valid;
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

    //connecting clients must supply a valid session cookie at websocket upgrade handshake negotiation time
    const char * headerkeys[] = { "Cookie" };
    size_t headerKeyCount = sizeof(headerkeys) / sizeof(char*);
    webSocket.onValidateHttpHeader(validateHttpHeader, headerkeys, headerKeyCount);
    webSocket.begin();
}

void loop() {
    webSocket.loop();
}

