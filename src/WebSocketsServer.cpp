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

WebSocketsServer::WebSocketsServer(uint16_t port) :
    _port(port), _server(new WEBSOCKETS_NETWORK_SERVER_CLASS(port)), _cbEvent(NULL)
{
}

WebSocketsServer::~WebSocketsServer() {
    // disconnect all clients
    disconnect();
    // TODO how to close server?
}

/**
 * calles to init the Websockets server
 */
void WebSocketsServer::begin(void) {
    // init client storage /////////---- not needed anymore, client are already initialize when created
 /*  for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        WSclient_t & client = _clients[i];
        client.num = WEBSOCKETS_SERVER_CLIENT_MAX; //if MAX -> means unconnected
        client.status = WSC_NOT_CONNECTED;
        client.tcp = NULL;
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
        client.isSSL = false;
        client.ssl = NULL;
#endif
        client.cUrl = "";
        client.cCode = 0;
        client.cKey = "";
        client.cProtocol = "";
        client.cVersion = 0;
        client.cIsUpgrade = false;
        client.cIsWebsocket = false;
    }*/

#ifdef ESP8266
    randomSeed(RANDOM_REG32);
#else
    // TODO find better seed
    randomSeed(millis());
#endif

    _server->begin();
#ifdef WS_SERVER_DEBUG
    WS_PRINTLN("[WS-Server] Server Started.");
#endif
}

/**
 * called in arduino loop
 */
void WebSocketsServer::loop(void) {
    //check if client, if it is a new client
    WSclient_t wsClient;
    wsClient._client = _server->available();

    //monitor for lost connect to clear socket
    if ( timeOutCounter > timeoutClient ) {
        for (uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
            clientIsConnected(_clients[i]);
        }
        timeOutCounter = 0;
    }
    timeOutCounter++;

    if (wsClient._client) {
        for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
            if( _clients[i]._client == wsClient._client ) { //not a new client, handle data
                handleClientData(_clients[i]);
                //todo ...---> add scedule sceck to see if client still connected ( ping pong ? )
                return;
            }
        }
        //if you got to that point, it is a new client
        for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
            if (!_clients[i]._client) { //if emty slot, assing new client then handle it.
                wsClient.num = i;
                _clients[i] = wsClient;
                _clients[i].tcp = &_clients[i]._client;
                handleNewClients(_clients[i]);
                return;
            }

        }
        wsClient._client.stop(); //close connection is no free spot
#ifdef WS_SERVER_DEBUG
        WS_PRINTLN("[WS-Server] No free socket, connection close");
#endif
    }
}

/**
 * set callback function
 * @param cbEvent WebSocketServerEvent
 */
void WebSocketsServer::onEvent(WebSocketServerEvent cbEvent) {
    _cbEvent = cbEvent;
}

/**
 * send text data to client
 * @param num uint8_t client id
 * @param payload uint8_t *
 * @param length size_t
 * @param headerToPayload bool  (see sendFrame for more details)
 */
void WebSocketsServer::sendTXT(uint8_t num, uint8_t * payload, size_t length, bool headerToPayload) {
    if(num >= WEBSOCKETS_SERVER_CLIENT_MAX) {
        return;
    }
    if(length == 0) {
        length = strlen((const char *) payload);
    }
    WSclient_t & client = _clients[num];
    if(clientIsConnected(client)) {
        sendFrame(client, WSop_text, payload, length, false, true, headerToPayload);
    }
}

void WebSocketsServer::sendTXT(uint8_t num, const uint8_t * payload, size_t length) {
    sendTXT(num, (uint8_t *) payload, length);
}

void WebSocketsServer::sendTXT(uint8_t num, char * payload, size_t length, bool headerToPayload) {
    sendTXT(num, (uint8_t *) payload, length, headerToPayload);
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
 * @param headerToPayload bool  (see sendFrame for more details)
 */
void WebSocketsServer::broadcastTXT(uint8_t * payload, size_t length, bool headerToPayload) {
    if(length == 0) {
        length = strlen((const char *) payload);
    }

    for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        WSclient_t & client = _clients[i];
        if(clientIsConnected(client)) {
            sendFrame(client, WSop_text, payload, length, false, true, headerToPayload);
        }
#ifdef ESP8266
        delay(0);
#endif
    }
}

void WebSocketsServer::broadcastTXT(const uint8_t * payload, size_t length) {
    broadcastTXT((uint8_t *) payload, length);
}

void WebSocketsServer::broadcastTXT(char * payload, size_t length, bool headerToPayload) {
    broadcastTXT((uint8_t *) payload, length, headerToPayload);
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
 * @param headerToPayload bool  (see sendFrame for more details)
 */
void WebSocketsServer::sendBIN(uint8_t num, uint8_t * payload, size_t length, bool headerToPayload) {
    if(num >= WEBSOCKETS_SERVER_CLIENT_MAX) {
        return;
    }
    WSclient_t & client = _clients[num];
    if(clientIsConnected(client)) {
        sendFrame(client, WSop_binary, payload, length, false, true, headerToPayload);
    }
}

void WebSocketsServer::sendBIN(uint8_t num, const uint8_t * payload, size_t length) {
    sendBIN(num, (uint8_t *) payload, length);
}

/**
 * send binary data to client all
 * @param payload uint8_t *
 * @param length size_t
 * @param headerToPayload bool  (see sendFrame for more details)
 */
void WebSocketsServer::broadcastBIN(uint8_t * payload, size_t length, bool headerToPayload) {
    for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        WSclient_t & client = _clients[i];
        if(clientIsConnected(client)) {
            sendFrame(client, WSop_binary, payload, length, false, true, headerToPayload);
        }
#ifdef ESP8266
        delay(0);
#endif
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
        WSclient_t & client = _clients[i];
        if(clientIsConnected(client)) {
            WebSockets::clientDisconnect(client, 1000);
        }
    }
}

/**
 * disconnect one client
 * @param num uint8_t client id
 */
void WebSocketsServer::disconnect(uint8_t num) {
    if(num >= WEBSOCKETS_SERVER_CLIENT_MAX) {
        return;
    }
    WSclient_t & client = _clients[num];
    if(clientIsConnected(client)) {
        WebSockets::clientDisconnect(client, 1000);
    }
}

#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
/**
 * get an IP for a client
 * @param num uint8_t client id
 * @return IPAddress
 */
IPAddress WebSocketsServer::remoteIP(uint8_t num) {
    if(num < WEBSOCKETS_SERVER_CLIENT_MAX) {
        WSclient_t * client = &_clients[num];
        if(clientIsConnected(*client)) {
            return client->tcp->remoteIP();
        }
    }

    return IPAddress();
}
#endif

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

void WebSocketsServer::messageReceived(WSclient_t & client, WSopcode_t opcode, uint8_t * payload, size_t lenght) {
    WStype_t type = WStype_ERROR;

    switch(opcode) {
        case WSop_text:
            type = WStype_TEXT;
            break;
        case WSop_binary:
            type = WStype_BIN;
            break;
    }

    runCbEvent(client.num, type, payload, lenght);
}
/**
 * Disconnect an client
 * @param client WSclient_t *  ptr to the client struct
 */
void WebSocketsServer::clientDisconnect(WSclient_t & client) {

#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
    if(client.isSSL && client.ssl) {
        if(client.ssl->connected()) {
            client.ssl->flush();
            client.ssl->stop();
        }
        delete client->ssl;
        client.ssl = NULL;
        client.tcp = NULL;
    }
#endif

    if(client._client) {
        if(client._client.connected()) {
            client._client.flush();
            client._client.stop();
        }
        //delete client->tcp;
        //client.tcp = NULL;
    }
    ////////////////////////////move to reset by initializer list
    /*client.cUrl = "";
    client.cKey = "";
    client.cProtocol = "";
    client.cVersion = 0;
    client.cIsUpgrade = false;
    client.cIsWebsocket = false;

    client.status = WSC_NOT_CONNECTED;*/
#ifdef WS_SERVER_DEBUG
    WS_PRINT("[WS-Server][");
    WS_PRINT(client.num);
    WS_PRINTLN("] client disconnected.");
#endif
    runCbEvent(client.num, WStype_DISCONNECTED, NULL, 0);
    //client.num = WEBSOCKETS_SERVER_CLIENT_MAX;
    client = WSclient_t(); //reset client to 0
}

/**
 * get client state
 * @param client WSclient_t *  ptr to the client struct
 * @return true = conneted
 */

bool WebSocketsServer::clientIsConnected(WSclient_t & client) {
    if (client.num >= WEBSOCKETS_SERVER_CLIENT_MAX) return false;
    if (!client._client) {
        clientDisconnect(client);
        return false;
    }

    if(client._client.connected()) {
        if(client.status != WSC_NOT_CONNECTED) {
            return true;
        }
    } else {
        // client lost
        if(client.status != WSC_NOT_CONNECTED) {
#ifdef WS_SERVER_DEBUG
            WS_PRINT("[WS-Server][");
            WS_PRINT(client.num);
            WS_PRINTLN("] client connection lost.");
#endif
            // do cleanup
            clientDisconnect(client);
        }
    }

    if(client._client) {
        // do cleanup
        clientDisconnect(client);
    }

    return false;
}

/**
 * Handle incomming Connection Request
 */

void WebSocketsServer::handleNewClients(WSclient_t & client) {
    bool ok = false;
    client.status = WSC_HEADER;
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)  //not shure for ESP8266
    while(_server->hasClient()) {
#endif 
        if(!clientIsConnected(client)) {
#ifdef WS_SERVER_DEBUG
            WS_PRINTLN("fail to make new connection");
#endif
            return;
        }

#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
                client.isSSL = false;
                client._client.setNoDelay(true);
#endif
                // set Timeout for readBytesUntil and readStringUntil
                client._client.setTimeout(WEBSOCKETS_TCP_TIMEOUT);

#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
                IPAddress ip = client._client.remoteIP();
#ifdef WS_SERVER_DEBUG
                WS_PRINT("[WS-Server][");
                WS_PRINT(client.num);
                WS_PRINT("] new client from ");
                WS_PRINT(ip[0]);
                WS_PRINT(".");
                WS_PRINT(ip[1]);
                WS_PRINT(".");
                WS_PRINT(ip[2]);
                WS_PRINT(".");
                WS_PRINTLN(ip[3]);
#endif
#else
#ifdef WS_SERVER_DEBUG
                WS_PRINT("[WS-Server][");
                WS_PRINT(client.num);
                WS_PRINTLN("] new client");
#endif
#endif
        
        ok = true;

#ifdef ESP8266
        delay(0);
#endif
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
    
#endif

}


void WebSocketsServer::handleClientData(WSclient_t & client) {
        if(clientIsConnected(client)) {
            int len = client._client.available();
            if(len > 0) {

                switch(client.status) {
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
#ifdef ESP8266
        delay(0);
#endif
}

/**
 * handle the WebSocket header reading
 * @param client WSclient_t *  ptr to the client struct
 */
void WebSocketsServer::handleHeader(WSclient_t & client) {

    String headerLine = client._client.readStringUntil('\n');
    headerLine.trim(); // remove \r

    if(headerLine.length() > 0) {
#ifdef WS_SERVER_DEBUG
        WS_PRINT("[WS-Server][");
        WS_PRINT(client.num);
        WS_PRINT("][handleHeader] RX: ");
        WS_PRINTLN(headerLine.c_str());
#endif
        // websocket request starts allways with GET see rfc6455
        if(headerLine.startsWith("GET ")) {
            // cut URL out
            client.cUrl = headerLine.substring(4, headerLine.indexOf(' ', 4));
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
            } else if(headerName.equalsIgnoreCase("Sec-WebSocket-Version")) {
                client.cVersion = headerValue.toInt();
            } else if(headerName.equalsIgnoreCase("Sec-WebSocket-Key")) {
                client.cKey = headerValue;
                client.cKey.trim(); // see rfc6455
            } else if(headerName.equalsIgnoreCase("Sec-WebSocket-Protocol")) {
                client.cProtocol = headerValue;
            } else if(headerName.equalsIgnoreCase("Sec-WebSocket-Extensions")) {
                client.cExtensions = headerValue;
            }
        } else {
#ifdef WS_SERVER_DEBUG
            WS_PRINT("[WS-Client][handleHeader] Header error (");
            WS_PRINT(headerLine.c_str());
            WS_PRINTLN(")");
#endif
        }

    } else {
#ifdef WS_SERVER_DEBUG
        WS_PRINT("[WS-Server][");
        WS_PRINT(client.num);
        WS_PRINTLN("][handleHeader] Header read fin.");
        WS_PRINT("[WS-Server][");
        WS_PRINT(client.num);
        WS_PRINT("][handleHeader]  - cURL: ");
        WS_PRINTLN(client.cUrl.c_str());
        WS_PRINT("[WS-Server][");
        WS_PRINT(client.num);
        WS_PRINT("][handleHeader]  - cIsUpgrade: ");
        WS_PRINTLN(client.cIsUpgrade);
        WS_PRINT("[WS-Server][");
        WS_PRINT(client.num);
        WS_PRINT("][handleHeader]  - cIsWebsocket: ");
        WS_PRINTLN(client.cIsWebsocket);
        WS_PRINT("[WS-Server][");
        WS_PRINT(client.num);
        WS_PRINT("][handleHeader]  - cKey: ");
        WS_PRINTLN(client.cKey.c_str());
        WS_PRINT("[WS-Server][");
        WS_PRINT(client.num);
        WS_PRINT("][handleHeader]  - cProtocol: ");
        WS_PRINTLN(client.cProtocol.c_str());
        WS_PRINT("[WS-Server][");
        WS_PRINT(client.num);
        WS_PRINT("][handleHeader]  - cExtensions: ");
        WS_PRINTLN(client.cExtensions.c_str());
        WS_PRINT("[WS-Server][");
        WS_PRINT(client.num);
        WS_PRINT("][handleHeader]  - cVersion: ");
        WS_PRINTLN(client.cVersion);
#endif
        bool ok = (client.cIsUpgrade && client.cIsWebsocket);

        if(ok) {
            if(client.cUrl.length() == 0) {
                ok = false;
            }
            if(client.cKey.length() == 0) {
                ok = false;
            }
            if(client.cVersion != 13) {
                ok = false;
            }
        }

        if(ok) {
#ifdef WS_SERVER_DEBUG
            WS_PRINT("[WS-Server][");
            WS_PRINT(client.num);
            WS_PRINTLN("][handleHeader] Websocket connection incomming.");
#endif
            // generate Sec-WebSocket-Accept key
            String sKey = acceptKey(client.cKey);
#ifdef WS_SERVER_DEBUG
            WS_PRINT("[WS-Server][");
            WS_PRINT(client.num);
            WS_PRINT("][handleHeader]  - sKey: ");
            WS_PRINTLN(sKey.c_str());
#endif
            client.status = WSC_CONNECTED;

            client._client.write("HTTP/1.1 101 Switching Protocols\r\n"
                    "Server: arduino-WebSocketsServer\r\n"
                    "Upgrade: websocket\r\n"
                    "Connection: Upgrade\r\n"
                    "Sec-WebSocket-Version: 13\r\n"
                    "Sec-WebSocket-Accept: ");
            client._client.write(sKey.c_str(), sKey.length());
            client._client.write("\r\n");

            if(client.cProtocol.length() > 0) {
                // TODO add api to set Protocol of Server
                client._client.write("Sec-WebSocket-Protocol: arduino\r\n");
            }

            // header end
            client._client.write("\r\n");

            // send ping
            WebSockets::sendFrame(client, WSop_ping);

            runCbEvent(client.num, WStype_CONNECTED, (uint8_t *) client.cUrl.c_str(), client.cUrl.length());

        } else {
            handleNonWebsocketConnection(client);
        }
    }
}
