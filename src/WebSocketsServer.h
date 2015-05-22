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



#define WEBSOCKETS_SERVER_CLIENT_MAX  (5)

typedef enum {
        WSC_NOT_CONNECTED,
        WSC_HEADER,
        WSC_CONNECTED
} WSclientsStatus_t;

typedef struct {
        uint8_t num; ///< connection number

        WSclientsStatus_t status;
#ifdef ESP8266
        WiFiClient tcp;
#else
#ifdef UIPETHERNET_H
        UIPClient tcp;
#else
        EthernetClient tcp;
#endif
#endif
        String cUrl;        ///< http url

        bool cIsUpgrade;    ///< Connection == Upgrade
        bool cIsWebsocket;  ///< Upgrade == websocket

        String cKey;        ///< client Sec-WebSocket-Key
        String cProtocol;   ///< client Sec-WebSocket-Protocol
        String cExtensions; ///< client Sec-WebSocket-Extensions
        int cVersion;       ///< client Sec-WebSocket-Version

        String sKey;        ///< server Sec-WebSocket-Key

} WSclients_t;

class WebSocketsServer {
public:
        WebSocketsServer(uint16_t port);
        ~WebSocketsServer(void);

        void begin(void);
        void loop(void);

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

        WSclients_t _clients[WEBSOCKETS_SERVER_CLIENT_MAX];


        void clientDisconnect(WSclients_t * client);
        bool clientIsConnected(WSclients_t * client);

        void handleNewClients(void);
        void handleClientData(void);

        void handleHeader(WSclients_t * client);


};



#endif /* WEBSOCKETSSERVER_H_ */
