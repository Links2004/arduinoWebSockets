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
    _server = new WEBSOCKETS_NETWORK_SERVER_CLASS(port);

    _cbEvent = NULL;

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
    WSclient_t * client;

    // init client storage
    for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        client = &_clients[i];

        client->num = i;
        client->status = WSC_NOT_CONNECTED;
        client->tcp = NULL;
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
        client->isSSL = false;
        client->ssl = NULL;
#endif
        client->cUrl = "";
        client->cCode = 0;
        client->cKey = "";
        client->cProtocol = "";
        client->cVersion = 0;
        client->cIsUpgrade = false;
        client->cIsWebsocket = false;
    }

#ifdef ESP8266
    randomSeed(RANDOM_REG32);
#else
    // TODO find better seed
    randomSeed(millis());
#endif

    _server->begin();

    DEBUG_WEBSOCKETS("[WS-Server] Server Started.\n");
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
    WSclient_t * client = &_clients[num];
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
    WSclient_t * client;
    if(length == 0) {
        length = strlen((const char *) payload);
    }

    for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        client = &_clients[i];
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
    WSclient_t * client = &_clients[num];
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
    WSclient_t * client;
    for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        client = &_clients[i];
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
        client = &_clients[i];
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
    WSclient_t * client = &_clients[num];
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
        if(clientIsConnected(client)) {
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

    runCbEvent(client->num, type, payload, lenght);

}

/**
 * Disconnect an client
 * @param client WSclient_t *  ptr to the client struct
 */
void WebSocketsServer::clientDisconnect(WSclient_t * client) {


#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
    if(client->isSSL && client->ssl) {
        if(client->ssl->connected()) {
            client->ssl->flush();
            client->ssl->stop();
        }
        delete client->ssl;
        client->ssl = NULL;
        client->tcp = NULL;
    }
#endif

    if(client->tcp) {
        if(client->tcp->connected()) {
            client->tcp->flush();
            client->tcp->stop();
        }
        delete client->tcp;
        client->tcp = NULL;
    }

    client->cUrl = "";
    client->cKey = "";
    client->cProtocol = "";
    client->cVersion = 0;
    client->cIsUpgrade = false;
    client->cIsWebsocket = false;

    client->status = WSC_NOT_CONNECTED;
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
    DEBUG_WEBSOCKETS("[WS-Server][%d] client disconnected.\n", client->num);
#else
    DEBUG_WEBSOCKETS("[WS-Server][");
    DEBUG_WEBSOCKETS(client->num);
    DEBUG_WEBSOCKETS("] client disconnected.\n");
#endif
    runCbEvent(client->num, WStype_DISCONNECTED, NULL, 0);

}

/**
 * get client state
 * @param client WSclient_t *  ptr to the client struct
 * @return true = conneted
 */
bool WebSocketsServer::clientIsConnected(WSclient_t * client) {

    if(!client->tcp) {
        return false;
    }

    if(client->tcp->connected()) {
        if(client->status != WSC_NOT_CONNECTED) {
            return true;
        }
    } else {
        // client lost
        if(client->status != WSC_NOT_CONNECTED) {
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
            DEBUG_WEBSOCKETS("[WS-Server][%d] client connection lost.\n", client->num);
#else
            DEBUG_WEBSOCKETS("[WS-Server][");
            DEBUG_WEBSOCKETS(client->num);
            DEBUG_WEBSOCKETS("] client connection lost.\n");
#endif
            
            // do cleanup
            clientDisconnect(client);
        }
    }

    if(client->tcp) {
        // do cleanup
        clientDisconnect(client);
    }

    return false;
}

/**
 * Handle incomming Connection Request
 */
void WebSocketsServer::handleNewClients(void) {
    WSclient_t * client;
    
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
    while(_server->hasClient()) {
#else
        WiFiClient clientContainer = _server->available();
        while(clientContainer){       
            tempIpAddressNewClient = clientContainer.remoteIP();

            for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
                client = &_clients[i];
                if(client->tcp)
                {
                    IPAddress ipContainer = client->tcp->remoteIP();
                    if(ipContainer[0]==tempIpAddressNewClient[0] && ipContainer[1]==tempIpAddressNewClient[1] && ipContainer[2]==tempIpAddressNewClient[2] && ipContainer[3]==tempIpAddressNewClient[3])
                    {
                        
                        return;
                    }
                }
            }
            
#endif
        bool ok = false;
        // search free list entry for client
       
        for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
            client = &_clients[i];
            
            DEBUG_WEBSOCKETS("looping for ");
            DEBUG_WEBSOCKETS(i);
            DEBUG_WEBSOCKETS(" times\n");
            // state is not connected or tcp connection is lost
            if(!clientIsConnected(client)) {
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
                // store new connection
                client->tcp = new WEBSOCKETS_NETWORK_CLASS(_server->available());
#else
                
                client->tcp = new WEBSOCKETS_NETWORK_CLASS(_server->available());//&clientContainer ;
//                client->tcp = new WEBSOCKETS_NETWORK_CLASS(clientContainer );
                IPAddress ip = client->tcp->remoteIP();
                if(ip[0]==0 && ip[1]==0 && ip[2]==0 && ip[3]==0)
                {
                    
                    return;
                }
#endif
                if(!client->tcp) {
                    DEBUG_WEBSOCKETS("[WS-Client] creating Network class failed!\n");
                    return;
                }

#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
                client->isSSL = false;
                client->tcp->setNoDelay(true);
#endif
                // set Timeout for readBytesUntil and readStringUntil
                client->tcp->setTimeout(WEBSOCKETS_TCP_TIMEOUT);
                client->status = WSC_HEADER;
                //IPAddress ip = client->tcp->remoteIP();
                
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
                
                DEBUG_WEBSOCKETS("[WS-Server][%d] new client from %d.%d.%d.%d\n", client->num, ip[0], ip[1], ip[2], ip[3]);
#else
                DEBUG_WEBSOCKETS("[WS-Server][");
                DEBUG_WEBSOCKETS( client->num);
                DEBUG_WEBSOCKETS("] new client from:");
                DEBUG_WEBSOCKETS(ip);
                DEBUG_WEBSOCKETS("\n");
#endif
                ok = true;
                break;
            }
        }

        if(!ok) {
            // no free space to handle client
            WEBSOCKETS_NETWORK_CLASS tcpClient = _server->available();
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
            IPAddress ip = client->tcp->remoteIP();
            DEBUG_WEBSOCKETS("[WS-Server] no free space new client from %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
#else
            DEBUG_WEBSOCKETS("[WS-Server] no free space new client\n");
#endif
            tcpClient.stop();
        }

#ifdef ESP8266
        delay(0);
#endif
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
    }
#else
    }
#endif
}

/**
 * Handel incomming data from Client
 */
void WebSocketsServer::handleClientData(void) {
//     Serial.println("handling client data");
    WSclient_t * client;
    for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        client = &_clients[i];
        if(clientIsConnected(client)) {
            int len = client->tcp->available();
            if(len > 0) {

                switch(client->status) {
                    case WSC_HEADER:
                        handleHeader(client);
                        break;
                    case WSC_CONNECTED:
                        WebSockets::handleWebsocket(client);
                        break;
                    case WSC_NOT_CONNECTED:
                        WebSockets::clientDisconnect(client, 1002);
                        break;
                    default:
                        
                        break;
                }
            }
        }
#ifdef ESP8266
        delay(0);
#endif
    }
}

/**
 * handle the WebSocket header reading
 * @param client WSclient_t *  ptr to the client struct
 */
void WebSocketsServer::handleHeader(WSclient_t * client) {

    String headerLine = client->tcp->readStringUntil('\n');
    headerLine.trim(); // remove \r

    if(headerLine.length() > 0) {
        #if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
            DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader] RX: %s\n", client->num, headerLine.c_str());
        #else
            DEBUG_WEBSOCKETS("[WS-Server][");
            DEBUG_WEBSOCKETS( client->num);
            DEBUG_WEBSOCKETS("][handleHeader] RX:");
            DEBUG_WEBSOCKETS(headerLine.c_str());
            DEBUG_WEBSOCKETS("\n");
        #endif
        // websocket request starts allways with GET see rfc6455
        if(headerLine.startsWith("GET ")) {
            // cut URL out
            client->cUrl = headerLine.substring(4, headerLine.indexOf(' ', 4));
        } else if(headerLine.indexOf(':')) {
            String headerName = headerLine.substring(0, headerLine.indexOf(':'));
            String headerValue = headerLine.substring(headerLine.indexOf(':') + 2);

            if(headerName.equalsIgnoreCase("Connection")) {
                if(headerValue.indexOf("Upgrade") >= 0) {
                    client->cIsUpgrade = true;
                }
            } else if(headerName.equalsIgnoreCase("Upgrade")) {
                if(headerValue.equalsIgnoreCase("websocket")) {
                    client->cIsWebsocket = true;
                }
            } else if(headerName.equalsIgnoreCase("Sec-WebSocket-Version")) {
                client->cVersion = headerValue.toInt();
            } else if(headerName.equalsIgnoreCase("Sec-WebSocket-Key")) {
                client->cKey = headerValue;
                client->cKey.trim(); // see rfc6455
            } else if(headerName.equalsIgnoreCase("Sec-WebSocket-Protocol")) {
                client->cProtocol = headerValue;
            } else if(headerName.equalsIgnoreCase("Sec-WebSocket-Extensions")) {
                client->cExtensions = headerValue;
            }
        } else {
             #if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
                DEBUG_WEBSOCKETS("[WS-Client][handleHeader] Header error (%s)\n", headerLine.c_str());
            #else
                DEBUG_WEBSOCKETS("[WS-Client][handleHeader] Header error (");
                DEBUG_WEBSOCKETS( headerLine.c_str());
                DEBUG_WEBSOCKETS(")\n");
            #endif
        }

    } else {
        #if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader] Header read fin.\n", client->num);

        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cURL: %s\n", client->num, client->cUrl.c_str());
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cIsUpgrade: %d\n", client->num, client->cIsUpgrade);
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cIsWebsocket: %d\n", client->num, client->cIsWebsocket);
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cKey: %s\n", client->num, client->cKey.c_str());
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cProtocol: %s\n", client->num, client->cProtocol.c_str());
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cExtensions: %s\n", client->num, client->cExtensions.c_str());
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - cVersion: %d\n", client->num, client->cVersion);
        #endif
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
            #if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
                        DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader] Websocket connection incomming.\n", client->num);
            #else
                        DEBUG_WEBSOCKETS("[WS-Server][");
                        DEBUG_WEBSOCKETS( client->num);
                        DEBUG_WEBSOCKETS("][handleHeader] Websocket connection incomming.\n");
            #endif
            

            // generate Sec-WebSocket-Accept key
            String sKey = acceptKey(client->cKey);

            
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
            DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader]  - sKey: %s\n", client->num, sKey.c_str());
#else
            DEBUG_WEBSOCKETS("[WS-Server][");
            DEBUG_WEBSOCKETS( client->num);
            DEBUG_WEBSOCKETS("][handleHeader]  - sKey: ");
            DEBUG_WEBSOCKETS( sKey.c_str());
            DEBUG_WEBSOCKETS("\n");
#endif
            client->status = WSC_CONNECTED;

            client->tcp->write("HTTP/1.1 101 Switching Protocols\r\n"
                    "Server: arduino-WebSocketsServer\r\n"
                    "Upgrade: websocket\r\n"
                    "Connection: Upgrade\r\n"
                    "Sec-WebSocket-Version: 13\r\n"
                    "Sec-WebSocket-Accept: ");
            client->tcp->write(sKey.c_str(), sKey.length());
            client->tcp->write("\r\n");

            if(client->cProtocol.length() > 0) {
                // TODO add api to set Protocol of Server
                client->tcp->write("Sec-WebSocket-Protocol: arduino\r\n");
            }

            // header end
            client->tcp->write("\r\n");

            // send ping
            WebSockets::sendFrame(client, WSop_ping);

            runCbEvent(client->num, WStype_CONNECTED, (uint8_t *) client->cUrl.c_str(), client->cUrl.length());

        } else {
            handleNonWebsocketConnection(client);
        }
    }
}



