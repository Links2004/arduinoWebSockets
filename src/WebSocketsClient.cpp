/**
 * @file WebSocketsClient.cpp
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
#include "WebSocketsClient.h"

WebSocketsClient::WebSocketsClient() {
    _cbEvent = NULL;
    _client.num = 0;

}

WebSocketsClient::~WebSocketsClient() {
    disconnect();
}

/**
 * calles to init the Websockets server
 */
void WebSocketsClient::begin(const char *host, uint16_t port, const char * url) {
    _host = host;
    _port = port;

    _client.status = WSC_NOT_CONNECTED;
    _client.cUrl = url;
    _client.cIsUpgrade = false;
    _client.cIsWebsocket = true;
    _client.cKey = "";
    _client.cProtocol = "";
    _client.cExtensions = "";
    _client.cVersion = 0;
}

void WebSocketsClient::begin(String host, uint16_t port, String url) {
    begin(host.c_str(), port, url.c_str());
}

/**
 * called in arduino loop
 */
void WebSocketsClient::loop(void) {
    if(!clientIsConnected(&_client)) {
        if(_client.tcp.connect(_host.c_str(), _port)) {
            DEBUG_WEBSOCKETS("[WS-Client] connected to %s:%u\n", _host.c_str(), _port);

            _client.tcp.setNoDelay(true);
            // set Timeout for readBytesUntil and readStringUntil
            _client.tcp.setTimeout(WEBSOCKETS_TCP_TIMEOUT);
            _client.status = WSC_HEADER;
        } else {
            DEBUG_WEBSOCKETS("[WS-Client] connection to %s:%u Faild\n", _host.c_str(), _port);
            delay(10); //some litle delay to not flood the server
        }
    } else {
        handleClientData();
    }
}

/**
 * set callback function
 * @param cbEvent WebSocketServerEvent
 */
void WebSocketsClient::onEvent(WebSocketClientEvent cbEvent) {
    _cbEvent = cbEvent;
}

/**
 * send text data to client
 * @param num uint8_t client id
 * @param payload uint8_t *
 * @param length size_t
 */
void WebSocketsClient::sendTXT(uint8_t * payload, size_t length) {
    if(length == 0) {
        length = strlen((const char *) payload);
    }
    if(clientIsConnected(&_client)) {
        sendFrame(&_client, WSop_text, payload, length);
    }
}

void WebSocketsClient::sendTXT(const uint8_t * payload, size_t length) {
    sendTXT((uint8_t *) payload, length);
}

void WebSocketsClient::sendTXT(char * payload, size_t length) {
    sendTXT((uint8_t *) payload, length);
}

void WebSocketsClient::sendTXT(const char * payload, size_t length) {
    sendTXT((uint8_t *) payload, length);
}

void WebSocketsClient::sendTXT(String payload) {
    sendTXT((uint8_t *) payload.c_str(), payload.length());
}

/**
 * send binary data to client
 * @param num uint8_t client id
 * @param payload uint8_t *
 * @param length size_t
 */
void WebSocketsClient::sendBIN(uint8_t * payload, size_t length) {
    if(clientIsConnected(&_client)) {
        sendFrame(&_client, WSop_binary, payload, length);
    }
}

void WebSocketsClient::sendBIN(const uint8_t * payload, size_t length) {
    sendBIN((uint8_t *) payload, length);
}

/**
 * disconnect one client
 * @param num uint8_t client id
 */
void WebSocketsClient::disconnect(void) {
    if(clientIsConnected(&_client)) {
        WebSockets::clientDisconnect(&_client, 1000);
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
void WebSocketsClient::messageRecived(WSclient_t * client, WSopcode_t opcode, uint8_t * payload, size_t lenght) {
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
        _cbEvent(type, payload, lenght);
    }
}

/**
 * Disconnect an client
 * @param client WSclient_t *  ptr to the client struct
 */
void WebSocketsClient::clientDisconnect(WSclient_t * client) {

    if(client->tcp) {
        client->tcp.flush();
        client->tcp.stop();
    }

    client->cKey = "";
    client->cProtocol = "";
    client->cVersion = 0;
    client->cIsUpgrade = false;
    client->cIsWebsocket = false;

    client->status = WSC_NOT_CONNECTED;

    DEBUG_WEBSOCKETS("[WS-Client] client disconnected.\n");

    if(_cbEvent) {
        _cbEvent(WStype_DISCONNECTED, NULL, 0);
    }
}

/**
 * get client state
 * @param client WSclient_t *  ptr to the client struct
 * @return true = conneted
 */
bool WebSocketsClient::clientIsConnected(WSclient_t * client) {

    if(client->status != WSC_NOT_CONNECTED && client->tcp.connected()) {
        return true;
    }

    if(client->status != WSC_NOT_CONNECTED) {
        // cleanup
        clientDisconnect(&_client);
    }
    return false;
}

/**
 * Handel incomming data from Client
 */
void WebSocketsClient::handleClientData(void) {
    int len = _client.tcp.available();
    if(len > 0) {
        switch(_client.status) {
            case WSC_HEADER:
                handleHeader(&_client);
                break;
            case WSC_CONNECTED:
                WebSockets::handleWebsocket(&_client);
                break;
            default:
                WebSockets::clientDisconnect(&_client, 1002);
                break;
        }
    }
    delay(0);
}

/**
 * handle the WebSocket header reading
 * @param client WSclient_t *  ptr to the client struct
 */
void WebSocketsClient::handleHeader(WSclient_t * client) {

    String headerLine = client->tcp.readStringUntil('\n');
    headerLine.trim(); // remove \r

    if(headerLine.length() > 0) {
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader] RX: %s\n", headerLine.c_str());

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
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader] Header read fin.\n");

        DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - cURL: %s\n", client->cUrl.c_str());
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - cIsUpgrade: %d\n", client->cIsUpgrade);
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - cIsWebsocket: %d\n", client->cIsWebsocket);
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - cKey: %s\n", client->cKey.c_str());
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - cProtocol: %s\n", client->cProtocol.c_str());
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - cExtensions: %s\n", client->cExtensions.c_str());
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - cVersion: %d\n", client->cVersion);

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

            DEBUG_WEBSOCKETS("[WS-Client][handleHeader] Websocket connection incomming.\n");

            // generate Sec-WebSocket-Accept key
            String sKey = acceptKey(client->cKey);

            DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - sKey: %s\n", sKey.c_str());

            client->status = WSC_CONNECTED;

            client->tcp.write("HTTP/1.1 101 Switching Protocols\r\n"
                    "Server: ESP8266-WebSocketsClient\r\n"
                    "Upgrade: websocket\r\n"
                    "Connection: Upgrade\r\n"
                    "Sec-WebSocket-Version: 13\r\n"
                    "Sec-WebSocket-Accept: ");
            client->tcp.write(sKey.c_str(), sKey.length());
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
                _cbEvent(WStype_CONNECTED, (uint8_t *) client->cUrl.c_str(), client->cUrl.length());
            }

        } else {
            DEBUG_WEBSOCKETS("[WS-Client][handleHeader] no Websocket connection close.\n");
            client->tcp.write("HTTP/1.1 400 Bad Request\r\n"
                    "Server: ESP8266-WebSocketsClient\r\n"
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

