/**
 * @file WebSockets.h
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

#ifndef WEBSOCKETS_H_
#define WEBSOCKETS_H_

#include <Arduino.h>

//#define DEBUG_WEBSOCKETS

//                      those macro have been added to save program space
//#define WS_DEBUG        //unmute this to display debug print from websockets.h and cpp
//#define WS_CLIENT_DEBUG //unmute this to display debug print from websocketsClient.h and cpp
//#define WS_SERVER_DEBUG //unmute this to display debug print from websocketsServer

#ifdef DEBUG_WEBSOCKETS
#define WS_PRINT(x) Serial.print(x);
#define WS_PRINTLN(x) Serial.println(x);
#else 
#define WS_PRINT(x)
#define WS_PRINTLN(x)
#define NODEBUG_WEBSOCKETS
#endif


        //Unmute to select Ethernet2 sheild library
#define W5500_H

#ifdef ESP8266
#define WEBSOCKETS_MAX_DATA_SIZE  (15*1024)
#define WEBSOCKETS_USE_BIG_MEM
#else
//atmega328p has only 2KB ram!
#define WEBSOCKETS_MAX_DATA_SIZE  (1024)
#endif

#define WEBSOCKETS_TCP_TIMEOUT    (1500)

#define NETWORK_ESP8266     (1)
#define NETWORK_W5100       (2)
#define NETWORK_ENC28J60    (3)
#define NETWORK_W5500       (4)


// select Network type based
#ifdef ESP8266
#define WEBSOCKETS_NETWORK_TYPE NETWORK_ESP8266
#elif defined W5500_H
#define WEBSOCKETS_NETWORK_TYPE NETWORK_W5500
#else
#define WEBSOCKETS_NETWORK_TYPE NETWORK_W5100
#endif


#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)

#ifndef ESP8266
#error "network type ESP8266 only possible on the ESP mcu!"
#endif

#include <ESP8266WiFi.h>
#define WEBSOCKETS_NETWORK_CLASS WiFiClient
#define WEBSOCKETS_NETWORK_SERVER_CLASS WiFiServer

#elif (WEBSOCKETS_NETWORK_TYPE == NETWORK_W5100)
#include <EthernetClient.h>
#include <EthernetServer.h>
#define WEBSOCKETS_NETWORK_CLASS EthernetClient
#define WEBSOCKETS_NETWORK_SERVER_CLASS EthernetServer

#elif (WEBSOCKETS_NETWORK_TYPE == NETWORK_ENC28J60)

#include <UIPEthernet.h>
#define WEBSOCKETS_NETWORK_CLASS UIPClient
#define WEBSOCKETS_NETWORK_SERVER_CLASS UIPServer

#elif (WEBSOCKETS_NETWORK_TYPE == NETWORK_W5500)
#include <EthernetClient.h>
#include <EthernetServer.h>
#define WEBSOCKETS_NETWORK_CLASS EthernetClient
#define WEBSOCKETS_NETWORK_SERVER_CLASS EthernetServer

#else
#error "no network type selected!"
#endif

typedef enum {
    WSC_NOT_CONNECTED,
    WSC_HEADER,
    WSC_CONNECTED
} WSclientsStatus_t;

typedef enum {
    WStype_ERROR,
    WStype_DISCONNECTED,
    WStype_CONNECTED,
    WStype_TEXT,
    WStype_BIN
} WStype_t;

typedef enum {
    WSop_continuation = 0x00, ///< %x0 denotes a continuation frame
    WSop_text = 0x01,         ///< %x1 denotes a text frame
    WSop_binary = 0x02,       ///< %x2 denotes a binary frame
                              ///< %x3-7 are reserved for further non-control frames
    WSop_close = 0x08,        ///< %x8 denotes a connection close
    WSop_ping = 0x09,         ///< %x9 denotes a ping
    WSop_pong = 0x0A          ///< %xA denotes a pong
                              ///< %xB-F are reserved for further control frames
} WSopcode_t;

struct WSclient_t {
    WSclient_t() : 
        num(-1),
        status(WSC_NOT_CONNECTED),
        tcp(NULL),
        cUrl(""),
        cCode(0),
        cKey(""),
        cProtocol(""),
        cVersion(0),
        cIsUpgrade(false),
        cIsWebsocket(false)
        {
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
        client.isSSL = false;
        client.ssl = NULL;
#endif  
        };
        uint8_t num; ///< connection number

        WSclientsStatus_t status;

        WEBSOCKETS_NETWORK_CLASS * tcp;
        WEBSOCKETS_NETWORK_CLASS _client;

#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
        bool isSSL;             ///< run in ssl mode
        WiFiClientSecure * ssl;
#endif

        String cUrl;        ///< http url
        uint16_t cCode;     ///< http code

        bool cIsUpgrade;    ///< Connection == Upgrade
        bool cIsWebsocket;  ///< Upgrade == websocket

        String cKey;        ///< client Sec-WebSocket-Key
        String cAccept;     ///< client Sec-WebSocket-Accept
        String cProtocol;   ///< client Sec-WebSocket-Protocol
        String cExtensions; ///< client Sec-WebSocket-Extensions
        uint16_t cVersion;  ///< client Sec-WebSocket-Version

};

class WebSockets {
    protected:
        virtual void clientDisconnect(WSclient_t & client);
        virtual bool clientIsConnected(WSclient_t & client);

        virtual void messageReceived(WSclient_t & client, WSopcode_t opcode, uint8_t * payload, size_t length);

        void clientDisconnect(WSclient_t & client, uint16_t code, char * reason = NULL, size_t reasonLen = 0);
        void sendFrame(WSclient_t & client, WSopcode_t opcode, uint8_t * payload = NULL, size_t length = 0, bool mask = false, bool fin = true, bool headerToPayload = false);

        void handleWebsocket(WSclient_t & client);

        bool readWait(WSclient_t & client, uint8_t *out, size_t n);

        String acceptKey(String clientKey);
        String base64_encode(uint8_t * data, size_t length);

};

#endif /* WEBSOCKETS_H_ */
