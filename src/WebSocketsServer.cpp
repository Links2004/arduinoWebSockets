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

    _cbEvent = NULL;

}

WebSocketsServer::~WebSocketsServer() {
    // todo how to close server?
}

/**
 * calles to init the Websockets server
 */
void WebSocketsServer::begin(void) {
    WSclient_t * client;

    // init client storage
    for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        client = &_clients[i];

        client->num = i;
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

/**
 * called in arduino loop
 */
void WebSocketsServer::loop(void) {
    handleNewClients();
    handleClientData();
}

/**
 * set callback function
 * @param cbEvent WebSocketServerEvent
 */
void WebSocketsServer::onEvent(WebSocketServerEvent cbEvent) {
    WebSocketsServer::_cbEvent = cbEvent;
}

/**
 * send text data to client
 * @param num uint8_t client id
 * @param payload uint8_t *
 * @param length size_t
 */
void WebSocketsServer::sendTXT(uint8_t num, uint8_t * payload, size_t length) {
    if(num >= WEBSOCKETS_SERVER_CLIENT_MAX) {
        return;
    }
    if(length == 0) {
        length = strlen((const char *) payload);
    }
    WSclient_t * client = &_clients[num];
    if(clientIsConnected(client)) {
        sendFrame(client, WSop_text, payload, length);
    }
}

void WebSocketsServer::sendTXT(uint8_t num, const uint8_t * payload, size_t length) {
    sendTXT(num, (uint8_t *) payload, length);
}

void WebSocketsServer::sendTXT(uint8_t num, char * payload, size_t length) {
    sendTXT(num, (uint8_t *) payload, length);
}

void WebSocketsServer::sendTXT(uint8_t num, const char * payload, size_t length) {
    sendTXT(num, (uint8_t *) payload, length);
}

void WebSocketsServer::sendTXT(uint8_t num, String payload) {
    sendTXT(num, (uint8_t *) payload.c_str(), payload.length());
}

/**
 * send text data to client all
 * @param payload uint8_t *
 * @param length size_t
 */
void WebSocketsServer::broadcastTXT(uint8_t * payload, size_t length) {
    WSclient_t * client;
    if(length == 0) {
        length = strlen((const char *) payload);
    }
    for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        client = &_clients[i];
        if(clientIsConnected(client)) {
            sendFrame(client, WSop_text, payload, length);
        }
    }
}

void WebSocketsServer::broadcastTXT(const uint8_t * payload, size_t length) {
    broadcastTXT((uint8_t *) payload, length);
}

void WebSocketsServer::broadcastTXT(char * payload, size_t length) {
    broadcastTXT((uint8_t *) payload, length);
}

void WebSocketsServer::broadcastTXT(const char * payload, size_t length) {
    broadcastTXT((uint8_t *) payload, length);
}

void WebSocketsServer::broadcastTXT(String payload) {
    broadcastTXT((uint8_t *) payload.c_str(), payload.length());
}

/**
 * send binary data to client
 * @param num uint8_t client id
 * @param payload uint8_t *
 * @param length size_t
 */
void WebSocketsServer::sendBIN(uint8_t num, uint8_t * payload, size_t length) {
    if(num >= WEBSOCKETS_SERVER_CLIENT_MAX) {
        return;
    }
    WSclient_t * client = &_clients[num];
    if(clientIsConnected(client)) {
        sendFrame(client, WSop_binary, payload, length);
    }
}

void WebSocketsServer::sendBIN(uint8_t num, const uint8_t * payload, size_t length) {
    sendBIN(num, (uint8_t *) payload, length);
}

/**
 * send binary data to client all
 * @param payload uint8_t *
 * @param length size_t
 */
void WebSocketsServer::broadcastBIN(uint8_t * payload, size_t length) {
    WSclient_t * client;
    for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        client = &_clients[i];
        if(clientIsConnected(client)) {
            sendFrame(client, WSop_binary, payload, length);
        }
    }
}

void WebSocketsServer::broadcastBIN(const uint8_t * payload, size_t length) {
    broadcastBIN((uint8_t *) payload, length);
}

/**
 * disconnect all clients
 */
void WebSocketsServer::disconnect(void) {
    WSclient_t * client;
    for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        client = &_clients[i];
        if(clientIsConnected(client)) {
            WebSockets::clientDisconnect(client, 1000);
        }
    }
}

/**
 * disconnect one client
 * @param num
 */
void WebSocketsServer::disconnect(uint8_t num) {
    if(num >= WEBSOCKETS_SERVER_CLIENT_MAX) {
        return;
    }
    WSclient_t * client = &_clients[num];
    if(clientIsConnected(client)) {
        WebSockets::clientDisconnect(client, 1000);
    }
}

//#################################################################################
//#################################################################################
//#################################################################################

/**
 *
 * @param client WSclient_t *  ptr to the client struct
 * @param opcode WSopcode_t
 * @param payload  uint8_t *
 * @param lenght size_t
 */
void WebSocketsServer::messageRecived(WSclient_t * client, WSopcode_t opcode, uint8_t * payload, size_t lenght) {
    WStype_t type = WStype_ERROR;

    switch(opcode) {
        case WSop_text:
            type = WStype_TEXT;
            break;
        case WSop_binary:
            type = WStype_BIN;
            break;
    }

    if(_cbEvent) {
        _cbEvent(client->num, type, payload, lenght);
    }
}

/**
 * Disconnect an client
 * @param client WSclient_t *  ptr to the client struct
 */
void WebSocketsServer::clientDisconnect(WSclient_t * client) {

    if(client->tcp) {
        client->tcp.stop();
    }

    client->cUrl = "";
    client->cKey = "";
    client->cProtocol = "";
    client->cVersion = 0;
    client->cIsUpgrade = false;
    client->cIsWebsocket = false;

    client->sKey = "";

    client->status = WSC_NOT_CONNECTED;

    DEBUG_WEBSOCKETS("[WS-Server][%d] client disconnected.\n", client->num);

    if(_cbEvent) {
        _cbEvent(client->num, WStype_DISCONNECTED, NULL, 0);
    }
}

/**
 * get client state
 * @param client WSclient_t *  ptr to the client struct
 * @return true = conneted
 */
bool WebSocketsServer::clientIsConnected(WSclient_t * client) {

    if(client->status != WSC_NOT_CONNECTED && client->tcp.connected()) {
        return true;
    }

    if(client->status != WSC_NOT_CONNECTED) {
        // cleanup
        clientDisconnect(client);
    }
    return false;
}

/**
 * Handle incomming Connection Request
 */
void WebSocketsServer::handleNewClients(void) {
    WSclient_t * client;
    while(_server->hasClient()) {
        bool ok = false;
        // search free list entry for client
        for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
            client = &_clients[i];

            // state is not connected or tcp connection is lost
            if(!clientIsConnected(client)) {

                // store new connection
                client->tcp = _server->available();
                client->tcp.setNoDelay(true);
                // set Timeout for readBytesUntil and readStringUntil
                client->tcp.setTimeout(WEBSOCKETS_TCP_TIMEOUT);
                client->status = WSC_HEADER;

                IPAddress ip = client->tcp.remoteIP();
                DEBUG_WEBSOCKETS("[WS-Server][%d] new client from %d.%d.%d.%d\n", client->num, ip[0], ip[1], ip[2], ip[3]);
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

    WSclient_t * client;
    for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        client = &_clients[i];
        if(clientIsConnected(client)) {
            int len = client->tcp.available();
            if(len > 0) {

                switch(client->status) {
                    case WSC_HEADER:
                        handleHeader(client);
                        break;
                    case WSC_CONNECTED:
                        WebSockets::handleWebsocket(client);
                        break;
                    default:
                        WebSockets::clientDisconnect(client, 1002);
                        break;
                }
            }
        }
        delay(0);
    }
}

/**
 * handle the WebSocket header reading
 * @param client WSclient_t *  ptr to the client struct
 */
void WebSocketsServer::handleHeader(WSclient_t * client) {

    String headerLine = client->tcp.readStringUntil('\n');
    headerLine.trim(); // remove \r

    if(headerLine.length() > 0) {
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader] RX: %s\n", client->num, headerLine.c_str());

        // websocket request starts allways with GET see rfc6455
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
            client->cKey.trim(); // see rfc6455
        } else if(headerLine.startsWith("Sec-WebSocket-Protocol: ")) {
            // 24 = lenght of "Sec-WebSocket-Protocol: "
            client->cProtocol = headerLine.substring(24);
        } else if(headerLine.startsWith("Sec-WebSocket-Extensions: ")) {
            // 26 = lenght of "Sec-WebSocket-Extensions: "
            client->cExtensions = headerLine.substring(26);
        }

    } else {
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader] Header read fin.\n", client->num);

        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cURL: %s\n", client->num, client->cUrl.c_str());
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cIsUpgrade: %d\n", client->num, client->cIsUpgrade);
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cIsWebsocket: %d\n", client->num, client->cIsWebsocket);
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cKey: %s\n", client->num, client->cKey.c_str());
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cProtocol: %s\n", client->num, client->cProtocol.c_str());
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cExtensions: %s\n", client->num, client->cExtensions.c_str());
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cVersion: %d\n", client->num, client->cVersion);

        bool ok = (client->cIsUpgrade && client->cIsWebsocket);

        if(ok) {
            if(client->cUrl.length() == 0) {
                ok = false;
            }
            if(client->cKey.length() == 0) {
                ok = false;
            }
            if(client->cVersion != 13) {
                ok = false;
            }
        }

        if(ok) {

            DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader] Websocket connection incomming.\n", client->num);

            // generate Sec-WebSocket-Accept key
            client->sKey = acceptKey(client->cKey);

            DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - sKey: %s\n", client->num, client->sKey.c_str());

            client->status = WSC_CONNECTED;

            client->tcp.write("HTTP/1.1 101 Switching Protocols\r\n"
                    "Server: ESP8266-WebSocketsServer\r\n"
                    "Upgrade: websocket\r\n"
                    "Connection: Upgrade\r\n"
                    "Sec-WebSocket-Version: 13\r\n"
                    "Sec-WebSocket-Accept: ");
            client->tcp.write(client->sKey.c_str(), client->sKey.length());
            client->tcp.write("\r\n");

            if(client->cProtocol.length() > 0) {
                // todo add api to set Protocol of Server
                client->tcp.write("Sec-WebSocket-Protocol: esp8266\r\n");
            }

            // header end
            client->tcp.write("\r\n");

            // send ping
            WebSockets::sendFrame(client, WSop_ping);

            if(_cbEvent) {
                _cbEvent(client->num, WStype_CONNECTED, (uint8_t *) client->cUrl.c_str(), client->cUrl.length());
            }

        } else {
            DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader] no Websocket connection close.\n", client->num);
            client->tcp.write("HTTP/1.1 400 Bad Request\r\n"
                    "Server: ESP8266-WebSocketsServer\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 32\r\n"
                    "Connection: close\r\n"
                    "Sec-WebSocket-Version: 13\r\n"
                    "\r\n"
                    "This is a Websocket server only!");
            clientDisconnect(client);
        }
    }
}

