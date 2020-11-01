
#ifndef __WEBSOCKETS4WEBSERVER_H
#define __WEBSOCKETS4WEBSERVER_H

#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>

#if WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266 && WEBSERVER_HAS_HOOK

class WebSockets4WebServer: public WebSocketsServerCore {
  public:

    WebSockets4WebServer(const String& origin = "", const String& protocol = "arduino"):
        WebSocketsServerCore(origin, protocol)
    {
    }

    ESP8266WebServer::HookFunction hookForWebserver (const String& wsRootDir, WebSocketServerEvent event)
    {
        onEvent(event);

        return [&, wsRootDir](const String & method, const String & url, WiFiClient * tcpClient, ESP8266WebServer::ContentTypeFunction contentType)
        {
printf("hook '%s' '%s' '%s'\n", method.c_str(), url.c_str(), wsRootDir.c_str());
            if (!(method == "GET" && url.indexOf(wsRootDir) == 0)) {
                return ESP8266WebServer::CLIENT_REQUEST_CAN_CONTINUE;
            }

            WSclient_t * client = handleNewClient(tcpClient);
            
            if (client)
            {
                // give "GET <url>"
                String headerLine;
                headerLine.reserve(url.length() + 5);
                headerLine = "GET ";
                headerLine += url;
                handleHeader(client, &headerLine);
            }
printf("conn? %i \n", tcpClient->connected());

            // tell webserver to not close but forget about this client
            return ESP8266WebServer::CLIENT_IS_GIVEN;
        };
    }
};

#endif // WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266 && WEBSERVER_HAS_HOOK

#endif // __WEBSOCKETS4WEBSERVER_H
