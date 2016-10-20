/* To compile using make CLI, create a folder under \firmware\user\applications and copy application.cpp there.
*  Then, copy src files under particleWebSocket folder.
*/

#include "application.h"
#include "particleWebSocket/WebSocketsClient.h"

WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length)
{
	switch (type)
	{
	case WStype_DISCONNECTED:
		Serial.printlnf("[WSc] Disconnected!");
		break;
	case WStype_CONNECTED:
		Serial.printlnf("[WSc] Connected to URL: %s", payload);
		webSocket.sendTXT("Connected\r\n");
		break;
	case WStype_TEXT:
		Serial.printlnf("[WSc] get text: %s", payload);
		break;
	case WStype_BIN:
		Serial.printlnf("[WSc] get binary length: %u", length);
		break;
	}
}

void setup()
{
	Serial.begin(9600);
	
	WiFi.setCredentials("[SSID]", "[PASSWORD]", WPA2, WLAN_CIPHER_AES_TKIP);
	WiFi.connect();
	    
	webSocket.begin("192.168.1.153", 85, "/ClientService/?variable=Test1212");
	webSocket.onEvent(webSocketEvent);
}

void loop()
{
	webSocket.sendTXT("Hello world!");
	delay(500);
	webSocket.loop();
}
