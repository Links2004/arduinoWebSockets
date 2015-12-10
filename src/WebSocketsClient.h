/**
 * @file WebSocketsClient.h
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

#ifndef WEBSOCKETSCLIENT_H_
#define WEBSOCKETSCLIENT_H_

#include <Arduino.h>
#include "WebSockets.h"

class WebSocketsClient: private WebSockets {
    public:

        typedef void (*WebSocketClientEvent)(WStype_t type, uint8_t * payload, size_t length);

        WebSocketsClient(void);
        ~WebSocketsClient(void);

        void begin(const char *host, uint16_t port, const char * url = "/");
        void begin(String host, uint16_t port, String url = "/");

#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
        void beginSSL(const char *host, uint16_t port, const char * url = "/");
        void beginSSL(String host, uint16_t port, String url = "/");
#endif

        void loop(void);

        void onEvent(WebSocketClientEvent cbEvent);

        void sendTXT(uint8_t * payload, size_t length = 0, bool headerToPayload = false);
        void sendTXT(const uint8_t * payload, size_t length = 0);
        void sendTXT(char * payload, size_t length = 0, bool headerToPayload = false);
        void sendTXT(const char * payload, size_t length = 0);
        void sendTXT(String payload);

        void sendBIN(uint8_t * payload, size_t length, bool headerToPayload = false);
        void sendBIN(const uint8_t * payload, size_t length);

        void disconnect(void);

    protected:
        String _host;
        uint16_t _port;

        WSclient_t _client;

        WebSocketClientEvent _cbEvent;

        void messageRecived(WSclient_t * client, WSopcode_t opcode, uint8_t * payload, size_t length);

        void clientDisconnect(WSclient_t * client);
        bool clientIsConnected(WSclient_t * client);

        void handleNewClients(void);
        void handleClientData(void);

        void sendHeader(WSclient_t * client);
        void handleHeader(WSclient_t * client);

        /**
         * called for sending a Event to the app
         * @param type WStype_t
         * @param payload uint8_t *
         * @param length size_t
         */
        virtual void runCbEvent(WStype_t type, uint8_t * payload, size_t length) {
            if(_cbEvent) {
                _cbEvent(type, payload, length);
            }
        }

};

#endif /* WEBSOCKETSCLIENT_H_ */
