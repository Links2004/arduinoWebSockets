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
#include "WebSockets.h"

#define WEBSOCKETS_SERVER_CLIENT_MAX  (5)




class WebSocketsServer: private WebSockets {
public:

#ifdef __AVR__
        typedef void (*WebSocketServerEvent)(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
#else
        typedef std::function<void (uint8_t num, WStype_t type, uint8_t * payload, size_t length)> WebSocketServerEvent;
#endif

        WebSocketsServer(uint16_t port, String origin = "", String protocol = "arduino");
        ~WebSocketsServer(void);

        void begin(void);

#if (WEBSOCKETS_NETWORK_TYPE != NETWORK_ESP8266_ASYNC)
        void loop(void);
#else
        // Async interface not need a loop call
        void loop(void) __attribute__ ((deprecated)) {}
#endif

        void onEvent(WebSocketServerEvent cbEvent);


        bool sendTXT(uint8_t num, uint8_t * payload, size_t length = 0, bool headerToPayload = false);
        bool sendTXT(uint8_t num, const uint8_t * payload, size_t length = 0);
        bool sendTXT(uint8_t num, char * payload, size_t length = 0, bool headerToPayload = false);
        bool sendTXT(uint8_t num, const char * payload, size_t length = 0);
        bool sendTXT(uint8_t num, String & payload);

        bool broadcastTXT(uint8_t * payload, size_t length = 0, bool headerToPayload = false);
        bool broadcastTXT(const uint8_t * payload, size_t length = 0);
        bool broadcastTXT(char * payload, size_t length = 0, bool headerToPayload = false);
        bool broadcastTXT(const char * payload, size_t length = 0);
        bool broadcastTXT(String & payload);

        bool sendBIN(uint8_t num, uint8_t * payload, size_t length, bool headerToPayload = false);
        bool sendBIN(uint8_t num, const uint8_t * payload, size_t length);

        bool broadcastBIN(uint8_t * payload, size_t length, bool headerToPayload = false);
        bool broadcastBIN(const uint8_t * payload, size_t length);

        void disconnect(void);
        void disconnect(uint8_t num);

        void setAuthorization(const char * user, const char * password);
        void setAuthorization(const char * auth);

#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266) || (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266_ASYNC)
        IPAddress remoteIP(uint8_t num);
#endif

protected:
        uint16_t _port;
        String _origin;
        String _protocol;
        String _base64Authorization; ///< Base64 encoded Auth request

        WEBSOCKETS_NETWORK_SERVER_CLASS * _server;

        WSclient_t _clients[WEBSOCKETS_SERVER_CLIENT_MAX];

        WebSocketServerEvent _cbEvent;

        bool newClient(WEBSOCKETS_NETWORK_CLASS * TCPclient);

        void messageRecived(WSclient_t * client, WSopcode_t opcode, uint8_t * payload, size_t length);

        void clientDisconnect(WSclient_t * client);
        bool clientIsConnected(WSclient_t * client);

#if (WEBSOCKETS_NETWORK_TYPE != NETWORK_ESP8266_ASYNC)
        void handleNewClients(void);
        void handleClientData(void);
#endif

        void handleHeader(WSclient_t * client, String * headerLine);


        /**
         * called if a non Websocket connection is coming in.
         * Note: can be override
         * @param client WSclient_t *  ptr to the client struct
         */
        virtual void handleNonWebsocketConnection(WSclient_t * client) {
            DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader] no Websocket connection close.\n", client->num);
            client->tcp->write("HTTP/1.1 400 Bad Request\r\n"
                    "Server: arduino-WebSocket-Server\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 32\r\n"
                    "Connection: close\r\n"
                    "Sec-WebSocket-Version: 13\r\n"
                    "\r\n"
                    "This is a Websocket server only!");
            clientDisconnect(client);
        }

        /**
         * called if a non Authorization connection is coming in.
         * Note: can be override
         * @param client WSclient_t *  ptr to the client struct
         */
        virtual void handleAuthorizationFailed(WSclient_t *client) {

            client->tcp->write("HTTP/1.1 401 Unauthorized\r\n"
                    "Server: arduino-WebSocket-Server\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 45\r\n"
                    "Connection: close\r\n"
                    "Sec-WebSocket-Version: 13\r\n"
                    "WWW-Authenticate: Basic realm=\"WebSocket Server\""
                    "\r\n"
                    "This Websocket server requires Authorization!");
            clientDisconnect(client);
        }

        /**
         * called for sending a Event to the app
         * @param num uint8_t
         * @param type WStype_t
         * @param payload uint8_t *
         * @param length size_t
         */
        virtual void runCbEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
            if(_cbEvent) {
                _cbEvent(num, type, payload, length);
            }
        }

};



#endif /* WEBSOCKETSSERVER_H_ */
