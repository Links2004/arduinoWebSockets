#include <Ethernet2.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp2.h>
/////////#include <HexDump.h> //downloaded from https://github.com/Yveaux/Arduino_HexDump

#include <WebSocketsServer.h>
//#include <Hash.h>

#include <Streaming.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF};
IPAddress ip(192, 168, 0, 110);

WebSocketsServer webSocket = WebSocketsServer(8081);

#define USE_SERIAL Serial

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
            //USE_SERIAL.printf("[%u] Disconnected!\n", num);
            USE_SERIAL << num << " Disconnected!" << endl;
            break;
        case WStype_CONNECTED:
            {
                //IPAddress ip = webSocket.remoteIP(num);
                //USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
				        USE_SERIAL << num << " Connected url: " << int(payload) << endl; 
				// send message to client
				webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            //USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
            USE_SERIAL << num << " get Text: " << int(payload) << endl;

            // send message to client
             webSocket.sendTXT(num, "message here");

            // send data to all connected clients
             webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            //USE_SERIAL.printf("[%u] get binary lenght: %u\n", num, lenght);
            USE_SERIAL << num <<" get binary lenght: " << lenght << endl;
            ///////////hexdump(payload, lenght);
            
            // send message to client
            webSocket.sendBIN(num, payload, lenght);
            break;
    }

}

void setup() {
    // USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);

    Ethernet.begin(mac, ip);

//   USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        //USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }


    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    webSocket.loop();
}
