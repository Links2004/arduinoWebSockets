/**
 * @file WebSocketsServer.cpp
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

#include "WebSockets.h"
#include "WebSocketsServer.h"

WebSocketsServer::WebSocketsServer(uint16_t port) {
    _port = port;
    _server = new WiFiServer(port);
}
WebSocketsServer::~WebSocketsServer() {
    // todo how to close server?
}

void WebSocketsServer::begin(void) {
    WSclients_t * client;

    // init client storage
    for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        client = &_clients[i];

        client->cUrl = "";
        client->cKey = "";
        client->cProtocol = "";
        client->cVersion = 0;
        client->cIsUpgrade = false;
        client->cIsWebsocket = false;

        client->sKey = "";

        client->status = WSC_NOT_CONNECTED;
    }

    _server->begin();
}

void WebSocketsServer::loop(void) {

    handleNewClients();

    handleClientData();

}

//#################################################################################
//#################################################################################
//#################################################################################

/**
 * Disconnect an client
 * @param num uint8_t index of _clients array
 */
void WebSocketsServer::clientDisconnect(uint8_t num) {
    WSclients_t * client = &_clients[num];

    if(client->tcp) {
        client->tcp.stop();
    }
    /*
     client->cUrl = "";
     client->cKey = "";
     client->cProtocol = "";
     client->cVersion = 0;
     client->cIsUpgrade = false;
     client->cIsWebsocket = false;

     client->sKey = "";
     */
    client->status = WSC_NOT_CONNECTED;

    DEBUG_WEBSOCKETS("[WS-Server][%d] client disconnected.\n", num);

}

/**
 * get client state
 * @param num uint8_t index of _clients array
 * @return true = conneted
 */
bool WebSocketsServer::clientIsConnected(uint8_t num) {
    WSclients_t * client = &_clients[num];

    if(client->status != WSC_NOT_CONNECTED && client->tcp.connected()) {
        return true;
    }

    if(client->status != WSC_NOT_CONNECTED) {
        // cleanup
        clientDisconnect(num);
    }
    return false;
}

/**
 * Handle incomming Connection Request
 */
void WebSocketsServer::handleNewClients(void) {
    WSclients_t * client;
    while(_server->hasClient()) {
        bool ok = false;
        // search free list entry for client
        for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
            client = &_clients[i];

            // state is not connected or tcp connection is lost
            if(!clientIsConnected(i)) {

                // store new connection
                client->tcp = _server->available();

                // set Timeout for readBytesUntil and readStringUntil
                client->tcp.setTimeout(1000);
                client->status = WSC_HEADER;

                IPAddress ip = client->tcp.remoteIP();
                DEBUG_WEBSOCKETS("[WS-Server][%d] new client from %d.%d.%d.%d\n", i, ip[0], ip[1], ip[2], ip[3]);
                ok = true;
                break;
            }
        }

        if(!ok) {
            // no free space to handle client
            WiFiClient tcpClient = _server->available();
            IPAddress ip = tcpClient.remoteIP();
            DEBUG_WEBSOCKETS("[WS-Server] no free space new client from %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
            tcpClient.stop();
        }

        delay(0);
    }
}

/**
 * Handel incomming data from Client
 */
void WebSocketsServer::handleClientData(void) {

    WSclients_t * client;
    for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        if(clientIsConnected(i)) {
            client = &_clients[i];
            int len = client->tcp.available();
            if(len > 0) {

                switch(client->status) {
                    case WSC_HEADER:
                        handleHeader(i);
                        break;
                    case WSC_CONNECTED:
                        break;
                    default:
                        clientDisconnect(i);
                        break;
                }
            }
        }
        delay(0);
    }
}


/*
 [WS-Server][0] new client from 192.168.2.23
 [WS-Server][0][handleHeader] RX: GET /test HTTP/1.1
 [WS-Server][0][handleHeader] RX: Host: 10.11.2.1:81
 [WS-Server][0][handleHeader] RX: Connection: Upgrade
 [WS-Server][0][handleHeader] RX: Pragma: no-cache
 [WS-Server][0][handleHeader] RX: Cache-Control: no-cache
 [WS-Server][0][handleHeader] RX: Upgrade: websocket
 [WS-Server][0][handleHeader] RX: Origin: null
 [WS-Server][0][handleHeader] RX: Sec-WebSocket-Version: 13
 [WS-Server][0][handleHeader] RX: DNT: 1
 [WS-Server][0][handleHeader] RX: User-Agent: Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/43.0.2357.65 Safari/537.36
 [WS-Server][0][handleHeader] RX: Accept-Encoding: gzip, deflate, sdch
 [WS-Server][0][handleHeader] RX: Accept-Language: de-DE,de;q=0.8,en-US;q=0.6,en;q=0.4
 [WS-Server][0][handleHeader] RX: Sec-WebSocket-Key: FRddfIKSePnzAzKOqUGI/Q==
 [WS-Server][0][handleHeader] RX: Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits
 [WS-Server][0][handleHeader] RX: Sec-WebSocket-Protocol: esp8266, test
 */

/**
 * handle the WebSocket headder reading
 * @param num uint8_t index of _clients array
 */
void WebSocketsServer::handleHeader(uint8_t num) {
    WSclients_t * client = &_clients[num];

    String headerLine = client->tcp.readStringUntil('\n');
    headerLine.trim(); // remove \r


    if(headerLine.length() > 0) {
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader] RX: %s\n", num, headerLine.c_str());

        // websocket request starts allway with GET see rfc6455
        if(headerLine.startsWith("GET ")) {
            // cut URL out
            client->cUrl = headerLine.substring(4, headerLine.indexOf(' ', 4));
        } else if(headerLine == "Connection: Upgrade") {
            client->cIsUpgrade = true;
        } else if(headerLine == "Upgrade: websocket") {
            client->cIsWebsocket = true;
        } else if(headerLine.startsWith("Sec-WebSocket-Version: ")) {
            // 23 = lenght of "Sec-WebSocket-Version: "
            client->cVersion = headerLine.substring(23).toInt();
        } else if(headerLine.startsWith("Sec-WebSocket-Key: ")) {
            // 19 = lenght of "Sec-WebSocket-Key: "
            client->cKey = headerLine.substring(19);
        } else if(headerLine.startsWith("Sec-WebSocket-Protocol: ")) {
            // 24 = lenght of "Sec-WebSocket-Protocol: "
            client->cProtocol = headerLine.substring(24);
        } else if(headerLine.startsWith("Sec-WebSocket-Extensions: ")) {
            // 26 = lenght of "Sec-WebSocket-Extensions: "
            client->cExtensions = headerLine.substring(26);
        }

    } else {
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader] Header read fin.\n", num);

        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cURL: %s\n", num, client->cUrl.c_str());
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cIsUpgrade: %d\n", num, client->cIsUpgrade);
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cIsWebsocket: %d\n", num, client->cIsWebsocket);
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cKey: %s\n", num, client->cKey.c_str());
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cProtocol: %s\n", num, client->cProtocol.c_str());
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cExtensions: %s\n", num, client->cExtensions.c_str());
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cVersion: %d\n", num, client->cVersion);

        bool ok = (client->cIsUpgrade && client->cIsWebsocket);

        if(ok) {
            if(client->cUrl.length() == 0) {
                ok = false;
            }
            if(client->cKey.length() == 0) {
                ok = false;
            }
            if(client->cVersion == 0) {
                ok = false;
            }
        }

        if(ok) {

            DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader] Websocket connection incomming.\n", num);

            //todo generate server key

            //client->sKey.c_str();


            //client->status = WSC_CONNECTED;

        } else {
            DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader] no Websocket connection close.\n", num);
            client->tcp.print(F("HTTP/1.1 400 Bad Request\r\n"
                    "Server: ESP8266-WebSocketsServer\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 32\r\n"
                    "Connection: close\r\n"
                    "Sec-WebSocket-Version: 13\r\n"
                    "\r\n"
                    "This is a Websocket server only!"));
            clientDisconnect(num);
        }
    }
}
