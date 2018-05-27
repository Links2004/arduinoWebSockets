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
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
    _fingerprint = "";
#endif

#ifdef ESP8266
    randomSeed(RANDOM_REG32);
#else
    // todo find better seed
    randomSeed(millis());
#endif
}

void WebSocketsClient::begin(String host, uint16_t port, String url) {
    begin(host.c_str(), port, url.c_str());
}

#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
void WebSocketsClient::beginSSL(const char *host, uint16_t port, const char * url, const char * fingerprint) {
    begin(host, port, url);
    _client.isSSL = true;
    _fingerprint = fingerprint;
}

void WebSocketsClient::beginSSL(String host, uint16_t port, String url, String fingerprint) {
    beginSSL(host.c_str(), port, url.c_str(), fingerprint.c_str());
}
#endif

/**
 * called in arduino loop
 */
void WebSocketsClient::loop(void) {
    if(!clientIsConnected(_client)) {

#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
        if(_client.isSSL) {
#ifdef WS_CLIENT_DEBUG
            WS_PRINTLN("[WS-Client] connect wss...");
#endif
            if(_client.ssl) {
                delete _client.ssl;
                _client.ssl = NULL;
            }
            _client.ssl = new WiFiClientSecure();
        } else {
#ifdef WS_CLIENT_DEBUG
            WS_PRINTLN("[WS-Client] connect ws...");
#endif
        }
#endif

        if(_client.tcp.connect(_host.c_str(), _port)) {
#ifdef WS_CLIENT_DEBUG
            WS_PRINT("[WS-Client] connected to ");
            WS_PRINT(_host.c_str());
            WS_PRINT(":");
            WS_PRINT(_port);
            WS_PRINTLN(".");
#endif
            _client.status = WSC_HEADER;

            // set Timeout for readBytesUntil and readStringUntil
            _client.tcp.setTimeout(WEBSOCKETS_TCP_TIMEOUT);

#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
            _client.tcp.setNoDelay(true);

            if(_client.isSSL && _fingerprint.length()) {
                if(!_client.ssl->verify(_fingerprint.c_str(), _host.c_str())) {
#ifdef WS_CLIENT_DEBUG
                    WS_PRINTLN("[WS-Client] creating Network class failed!");
#endif
                    WebSockets::clientDisconnect(&_client, 1000);
                    return;
                }
            }
#endif

            // send Header to Server
            sendHeader(_client);

        } else {
#ifdef WS_CLIENT_DEBUG
            WS_PRINT("[WS-Client] connection to ");
            WS_PRINT(_host.c_str());
            WS_PRINT(":");
            WS_PRINT(_port);
            WS_PRINTLN(" Faild");
#endif
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
 * @param headerToPayload bool  (see sendFrame for more details)
 */
void WebSocketsClient::sendTXT(uint8_t * payload, size_t length, bool headerToPayload) {
    if(length == 0) {
        length = strlen((const char *) payload);
    }
    if(clientIsConnected(_client)) {
        sendFrame(_client, WSop_text, payload, length, true, true, headerToPayload);
    }
}

void WebSocketsClient::sendTXT(const uint8_t * payload, size_t length) {
    sendTXT((uint8_t *) payload, length);
}

void WebSocketsClient::sendTXT(char * payload, size_t length, bool headerToPayload) {
    sendTXT((uint8_t *) payload, length, headerToPayload);
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
 * @param headerToPayload bool  (see sendFrame for more details)
 */
void WebSocketsClient::sendBIN(uint8_t * payload, size_t length, bool headerToPayload) {
    if(clientIsConnected(_client)) {
        sendFrame(_client, WSop_binary, payload, length, true, true, headerToPayload);
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
    if(clientIsConnected(_client)) {
        WebSockets::clientDisconnect(_client, 1000);
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
void WebSocketsClient::messageReceived(WSclient_t & client, WSopcode_t opcode, uint8_t * payload, size_t length) {
    WStype_t type = WStype_ERROR;

    switch(opcode) {
        case WSop_text:
            type = WStype_TEXT;
            break;
        case WSop_binary:
            type = WStype_BIN;
            break;
    }
    runCbEvent(type, payload, length);
}

/**
 * Disconnect an client
 * @param client WSclient_t *  ptr to the client struct
 */
void WebSocketsClient::clientDisconnect(WSclient_t & client) {
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
    if(client.isSSL && client->ssl) {
        if(client.ssl->connected()) {
            client.ssl->flush();
            client.ssl->stop();
        }
        delete client.ssl;
    }
#endif
    if(client.tcp) {
        if(client.tcp.connected()) {
            client.tcp.flush();
            client.tcp.stop();
        }
    }

    client = WSclient_t(); //reset client to 0
#ifdef WS_CLIENT_DEBUG
    WS_PRINTLN("[WS-Client] client disconnected.");
#endif
    runCbEvent(WStype_DISCONNECTED, NULL, 0);
}

/**
 * get client state
 * @param client WSclient_t *  ptr to the client struct
 * @return true = conneted
 */
bool WebSocketsClient::clientIsConnected(WSclient_t & client) {
    if(!client.tcp) {
        return false;
    }
    if(client.tcp.connected()) {
        if(client.status != WSC_NOT_CONNECTED) {
            return true;
        }
    } else {
        // client lost
        if(client.status != WSC_NOT_CONNECTED) {
#ifdef WS_CLIENT_DEBUG
            WS_PRINTLN("[WS-Client] connection lost.");
#endif
            // do cleanup
            clientDisconnect(client);
        }
    }
    if(client.tcp) {
        // do cleanup
        clientDisconnect(client);
    }
    return false;
}

/**
 * Handel incomming data from Client
 */
void WebSocketsClient::handleClientData(void) {
#ifdef WS_CLIENT_DEBUG
    WS_PRINTLN("handle client data");
#endif
    int len = _client.tcp.available();
    if(len > 0) {
        switch(_client.status) {
            case WSC_HEADER:
                handleHeader(_client);
                WS_PRINTLN("data:handleHeader");
                break;
            case WSC_CONNECTED:
                WebSockets::handleWebsocket(_client);
                WS_PRINTLN("data: handle WS");
                break;
            default:
                WebSockets::clientDisconnect(_client, 1002);
                WS_PRINTLN("data: disc");
                break;
        }
    }
#ifdef ESP8266
    delay(0);
#endif
}

/**
 * send the WebSocket header to Server
 * @param client WSclient_t *  ptr to the client struct
 */
void WebSocketsClient::sendHeader(WSclient_t & client) {
#ifdef WS_CLIENT_DEBUG
    WS_PRINTLN("[WS-Client][sendHeader] sending header...");
#endif
    uint8_t randomKey[16] = { 0 };

    for(uint8_t i = 0; i < sizeof(randomKey); i++) {
        randomKey[i] = random(0xFF);
    }

    client.cKey = base64_encode(&randomKey[0], 16);

#ifndef NODEBUG_WEBSOCKETS
    unsigned long start = micros();
#endif

    String handshake =  "GET " + client.cUrl + " HTTP/1.1\r\n"
                        "Host: " + _host + "\r\n"
                        "Upgrade: websocket\r\n"
                        "Connection: Upgrade\r\n"
                        "User-Agent: arduino-WebSocket-Client\r\n"
                        "Sec-WebSocket-Version: 13\r\n"
                        "Sec-WebSocket-Protocol: arduino\r\n"
                        "Sec-WebSocket-Key: " + client.cKey + "\r\n";

    if(client.cExtensions.length() > 0) {
        handshake += "Sec-WebSocket-Extensions: " + client.cExtensions + "\r\n";
    }

    handshake += "\r\n";

    client.tcp.write(handshake.c_str(), handshake.length());
#ifdef WS_CLIENT_DEBUG
    WS_PRINT("[WS-Client][sendHeader] sending header... Done (");
    WS_PRINT((micros() - start));
    WS_PRINTLN("us).");
#endif
}

/**
 * handle the WebSocket header reading
 * @param client WSclient_t *  ptr to the client struct
 */
void WebSocketsClient::handleHeader(WSclient_t & client) {
    String headerLine = client.tcp.readStringUntil('\n');
    headerLine.trim(); // remove \r

    if(headerLine.length() > 0) {
#ifdef WS_CLIENT_DEBUG
        WS_PRINT("[WS-Client][handleHeader] RX: ");
        WS_PRINTLN(headerLine.c_str());
#endif
        if(headerLine.startsWith("HTTP/1.")) {
            // "HTTP/1.1 101 Switching Protocols"
            client.cCode = headerLine.substring(9, headerLine.indexOf(' ', 9)).toInt();
        } else if(headerLine.indexOf(':')) {
            String headerName = headerLine.substring(0, headerLine.indexOf(':'));
            String headerValue = headerLine.substring(headerLine.indexOf(':') + 2);

            if(headerName.equalsIgnoreCase("Connection")) {
                if(headerValue.indexOf("Upgrade") >= 0) {
                    client.cIsUpgrade = true;
                }
            } else if(headerName.equalsIgnoreCase("Upgrade")) {
                if(headerValue.equalsIgnoreCase("websocket")) {
                    client.cIsWebsocket = true;
                }
            } else if(headerName.equalsIgnoreCase("Sec-WebSocket-Accept")) {
                client.cAccept = headerValue;
                client.cAccept.trim(); // see rfc6455
            } else if(headerName.equalsIgnoreCase("Sec-WebSocket-Protocol")) {
                client.cProtocol = headerValue;
            } else if(headerName.equalsIgnoreCase("Sec-WebSocket-Extensions")) {
                client.cExtensions = headerValue;
            } else if(headerName.equalsIgnoreCase("Sec-WebSocket-Version")) {
                client.cVersion = headerValue.toInt();
            }
        } else {
#ifdef WS_CLIENT_DEBUG
            WS_PRINT("[WS-Client][handleHeader] Header error (");
            WS_PRINT(headerLine.c_str());
            WS_PRINTLN(")");
#endif
        }

    } else {
#ifdef WS_CLIENT_DEBUG
        WS_PRINTLN("[WS-Client][handleHeader] Header read fin.");
        WS_PRINTLN("[WS-Client][handleHeader] Client settings:");
        WS_PRINT("[WS-Client][handleHeader]  - cURL: ");
        WS_PRINTLN(client.cUrl.c_str());
        WS_PRINTLN("[WS-Client][handleHeader]  - cKey: ");
        WS_PRINTLN(client.cKey.c_str());
        WS_PRINTLN("[WS-Client][handleHeader] Server header:");
        WS_PRINT("[WS-Client][handleHeader]  - cCode: ");
        WS_PRINTLN(client.cCode);
        WS_PRINT("[WS-Client][handleHeader]  - cIsUpgrade: ");
        WS_PRINTLN(client.cIsUpgrade);
        WS_PRINT("[WS-Client][handleHeader]  - cIsWebsocket: ");
        WS_PRINTLN(client.cIsWebsocket);
        WS_PRINT("[WS-Client][handleHeader]  - cAccept: ");
        WS_PRINTLN(client.cAccept.c_str());
        WS_PRINT("[WS-Client][handleHeader]  - cProtocol: ");
        WS_PRINTLN(client.cProtocol.c_str());
        WS_PRINT("[WS-Client][handleHeader]  - cExtensions: ");
        WS_PRINTLN(client.cExtensions.c_str());
        WS_PRINT("[WS-Client][handleHeader]  - cVersion: ");
        WS_PRINTLN(client.cVersion);
#endif
        bool ok = (client.cIsUpgrade && client.cIsWebsocket);

        if(ok) {
            switch(client.cCode) {
                case 101:  ///< Switching Protocols

                    break;
                case 403: ///< Forbidden
                    // todo handle login
                default:   ///< Server dont unterstand requrst
                    ok = false;
#ifdef WS_CLIENT_DEBUG
                    WS_PRINT("[WS-Client][handleHeader] serverCode is not 101 (");
                    WS_PRINT(client.cCode);
                    WS_PRINTLN(")");
#endif
                    clientDisconnect(client);
                    break;
            }
        }

        if(ok) {
            if(client.cAccept.length() == 0) {
                ok = false;
            } else {
                // generate Sec-WebSocket-Accept key for check
                String sKey = acceptKey(client.cKey);
                if(sKey != client.cAccept) {
#ifdef WS_CLIENT_DEBUG
                    WS_PRINTLN("[WS-Client][handleHeader] Sec-WebSocket-Accept is wrong");
#endif
                    ok = false;
                }
            }
        }

        if(ok) {
#ifdef WS_CLIENT_DEBUG
            WS_PRINTLN("[WS-Client][handleHeader] Websocket connection init done.");
#endif
            client.status = WSC_CONNECTED;

            runCbEvent(WStype_CONNECTED, (uint8_t *) client.cUrl.c_str(), client.cUrl.length());

        } else {
#ifdef WS_CLIENT_DEBUG
            WS_PRINTLN("[WS-Client][handleHeader] no Websocket connection close.");
#endif
            client.tcp.write("This is a webSocket client!");
            clientDisconnect(client);
        }
    }
}
