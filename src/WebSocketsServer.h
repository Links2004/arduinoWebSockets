/**
 * @file WebSocketsServer.h
 * @date 20.05.2015
 * @author Markus Sattler
 *
 * Copyright (c) 2015 Markus Sattler. All rights reserved.
 * This file is part of the WebSockets for Arduino.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef WEBSOCKETSSERVER_H_
#define WEBSOCKETSSERVER_H_

#include <Arduino.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <UIPEthernet.h>
#ifndef UIPETHERNET_H
#include <Ethernet.h>
#include <SPI.h>
#endif
#endif

#include "WebSockets.h"

#define WEBSOCKETS_SERVER_CLIENT_MAX  (5)


typedef enum {
    WStype_ERROR,
    WStype_DISCONNECTED,
    WStype_CONNECTED,
    WStype_TEXT,
    WStype_BIN
} WStype_t;


class WebSocketsServer: private WebSockets {
public:

        typedef void (*WebSocketServerEvent)(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

        WebSocketsServer(uint16_t port);
        ~WebSocketsServer(void);

        void begin(void);
        void loop(void);

        void onEvent(WebSocketServerEvent cbEvent);


        void sendTXT(uint8_t num, uint8_t * payload, size_t length = 0);
        void sendTXT(uint8_t num, const uint8_t * payload, size_t length = 0);
        void sendTXT(uint8_t num, char * payload, size_t length = 0);
        void sendTXT(uint8_t num, const char * payload, size_t length = 0);
        void sendTXT(uint8_t num, String payload);

        void broadcastTXT(uint8_t * payload, size_t length = 0);
        void broadcastTXT(const uint8_t * payload, size_t length = 0);
        void broadcastTXT(char * payload, size_t length = 0);
        void broadcastTXT(const char * payload, size_t length = 0);
        void broadcastTXT(String payload);

        void sendBIN(uint8_t num, uint8_t * payload, size_t length);
        void sendBIN(uint8_t num, const uint8_t * payload, size_t length);

        void broadcastBIN(uint8_t * payload, size_t length);
        void broadcastBIN(const uint8_t * payload, size_t length);

        void disconnect(void);
        void disconnect(uint8_t num);

        IPAddress remoteIP(uint8_t num);

private:
        uint16_t _port;

#ifdef ESP8266
        WiFiServer * _server;
#else
#ifdef UIPETHERNET_H
        UIPServer * _server;
#else
        EthernetServer * _server;
#endif
#endif

        WSclient_t _clients[WEBSOCKETS_SERVER_CLIENT_MAX];

        WebSocketServerEvent _cbEvent;

        void messageRecived(WSclient_t * client, WSopcode_t opcode, uint8_t * payload, size_t length);
        void clientConnected(WSclient_t * client);

        void clientDisconnect(WSclient_t * client);
        bool clientIsConnected(WSclient_t * client);

        void handleNewClients(void);
        void handleClientData(void);

        void handleHeader(WSclient_t * client);
        void handleWebsocket(WSclient_t * client);

};



#endif /* WEBSOCKETSSERVER_H_ */
