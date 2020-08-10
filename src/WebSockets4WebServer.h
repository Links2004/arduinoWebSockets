
#ifndef __WEBSOCKETS4WEBSERVER_H
#define __WEBSOCKETS4WEBSERVER_H

#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>

#if WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266

class WebSockets4WebServer: public WebSocketsServerCore {
  public:    

    WebSockets4WebServer(const String& origin = "", const String& protocol = "arduino"):
        WebSocketsServerCore(origin, protocol)
    {
    }

    ESP8266WebServer::HookFunction hookForWebserver (WebSocketServerEvent event);

};

#endif // WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266

#endif // __WEBSOCKETS4WEBSERVER_H
