/*
 * SocketIOclient.cpp
 *
 *  Created on: May 12, 2018
 *      Author: links
 */

#include "WebSockets.h"
#include "WebSocketsClient.h"
#include "SocketIOclient.h"

SocketIOclient::SocketIOclient() {

}

SocketIOclient::~SocketIOclient() {

}

void SocketIOclient::begin(const char *host, uint16_t port, const char * url, const char * protocol) {
    WebSocketsClient::beginSocketIO(host, port, url, protocol);
}

void SocketIOclient::begin(String host, uint16_t port, String url, String protocol) {
    WebSocketsClient::beginSocketIO(host, port, url, protocol);
}

void SocketIOclient::loop(void) {
    WebSocketsClient::loop();
    unsigned long t = millis();
    if((t - _lastConnectionFail) > EIO_HEARTBEAT_INTERVAL) {
        _lastConnectionFail = t;
        sendTXT(eIOtype_PING);
    }
}

void SocketIOclient::runCbEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            DEBUG_WEBSOCKETS("[wsIOc] Disconnected!\n");
            break;
        case WStype_CONNECTED: {
            DEBUG_WEBSOCKETS("[wsIOc] Connected to url: %s\n", payload);
            // send message to server when Connected
            // Engine.io upgrade confirmation message (required)
            sendTXT(eIOtype_UPGRADE);
        }
            break;
        case WStype_TEXT: {

            if(length < 1) {
                break;
            }

            engineIOmessageType_t eType = (engineIOmessageType_t) payload[0];
            switch(eType) {
                case eIOtype_PING:
                    payload[0] = eIOtype_PONG;
                    DEBUG_WEBSOCKETS("[wsIOc] get ping send pong (%s)\n", payload);
                    sendTXT(payload, length);
                    break;
                case eIOtype_PONG:
                    DEBUG_WEBSOCKETS("[wsIOc] get pong\n");
                    break;
                case eIOtype_OPEN:
                case eIOtype_CLOSE:
                case eIOtype_MESSAGE:
                case eIOtype_UPGRADE:
                case eIOtype_NOOP:
                default:
                    DEBUG_WEBSOCKETS("[wsIOc] Engine.IO Message Type %c (%02X) is not implemented\n", eType, eType);
                    DEBUG_WEBSOCKETS("[wsIOc] get text: %s\n", payload);
                    break;
            }

            // send message to server
            // webSocket.sendTXT("message here");
        }
            break;
        case WStype_BIN:
            DEBUG_WEBSOCKETS("[wsIOc] get binary length: %u\n", length);
            // hexdump(payload, length);

            // send data to server
            // webSocket.sendBIN(payload, length);
            break;
    }
}
