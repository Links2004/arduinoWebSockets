#include <Ethernet2.h>
#include <WebSocketsServer.h>
#include <Streaming.h>

#define USE_SERIAL Serial
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF};
IPAddress ip(192, 168, 0, 110);

WebSocketsServer webSocket = WebSocketsServer(8081);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

  switch (type) {
    case WStype_DISCONNECTED:
      USE_SERIAL << num << " Disconnected!" << endl;
      break;
    case WStype_CONNECTED:
      {
        //IPAddress ip = webSocket.remoteIP(num);
        USE_SERIAL << num << " Connected url: " << (char*)payload << endl;
        // send message to client
        webSocket.sendTXT(num, "Connected");
      }
      break;
    case WStype_TEXT:
      USE_SERIAL << num << " get Text: " << (char*)payload << endl;

      // send message to client
      webSocket.sendTXT(num, "message here");

      // send data to all connected clients
      webSocket.broadcastTXT("broadcast message here");
      break;
    case WStype_BIN:
      USE_SERIAL << num << " get binary lenght: ";

      // send message to client
      webSocket.sendBIN(num, payload, lenght);
      break;
  }
}

void setup() {
  USE_SERIAL.begin(115200);
  Ethernet.begin(mac, ip);
  USE_SERIAL.println("\n\n");
  USE_SERIAL.println(Ethernet.localIP());

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.flush();
    delay(1000);
  }

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
}
