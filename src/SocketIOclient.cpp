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

void SocketIOclient::begin(const char * host, uint16_t port, const char * url, const char * protocol) {
    WebSocketsClient::beginSocketIO(host, port, url, protocol);
}

void SocketIOclient::begin(String host, uint16_t port, String url, String protocol) {
    WebSocketsClient::beginSocketIO(host, port, url, protocol);
}

bool SocketIOclient::isConnected(void) {
    return WebSocketsClient::isConnected();
}

/**
 * send text data to client
 * @param num uint8_t client id
 * @param payload uint8_t *
 * @param length size_t
 * @param headerToPayload bool  (see sendFrame for more details)
 * @return true if ok
 */
bool SocketIOclient::sendEVENT(uint8_t * payload, size_t length, bool headerToPayload) {
    bool ret = false;
    if(length == 0) {
        length = strlen((const char *)payload);
    }
    if(clientIsConnected(&_client)) {
        if(!headerToPayload) {
            // webSocket Header
            ret = WebSocketsClient::sendFrameHeader(&_client, WSop_text, length + 2, true);
            // Engine.IO / Socket.IO Header
            if(ret) {
                uint8_t buf[3] = { eIOtype_MESSAGE, sIOtype_EVENT, 0x00 };
                ret            = WebSocketsClient::write(&_client, buf, 2);
            }
            if(ret) {
                ret = WebSocketsClient::write(&_client, payload, length);
            }
            return ret;
        } else {
            // TODO implement
        }

        // return WebSocketsClient::sendFrame(&_client, WSop_text, payload, length, true, true, headerToPayload);
    }
    return false;
}

bool SocketIOclient::sendEVENT(const uint8_t * payload, size_t length) {
    return sendEVENT((uint8_t *)payload, length);
}

bool SocketIOclient::sendEVENT(char * payload, size_t length, bool headerToPayload) {
    return sendEVENT((uint8_t *)payload, length, headerToPayload);
}

bool SocketIOclient::sendEVENT(const char * payload, size_t length) {
    return sendEVENT((uint8_t *)payload, length);
}

bool SocketIOclient::sendEVENT(String & payload) {
    return sendEVENT((uint8_t *)payload.c_str(), payload.length());
}

void SocketIOclient::loop(void) {
    WebSocketsClient::loop();
    unsigned long t = millis();
    if((t - _lastConnectionFail) > EIO_HEARTBEAT_INTERVAL) {
        _lastConnectionFail = t;
        //WebSocketsClient::sendTXT(eIOtype_PING);
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
            WebSocketsClient::sendTXT(eIOtype_UPGRADE);
        } break;
        case WStype_TEXT: {
            if(length < 1) {
                break;
            }

            engineIOmessageType_t eType = (engineIOmessageType_t)payload[0];
            switch(eType) {
                case eIOtype_PING:
                    payload[0] = eIOtype_PONG;
                    DEBUG_WEBSOCKETS("[wsIOc] get ping send pong (%s)\n", payload);
                    WebSocketsClient::sendTXT(payload, length, false);
                    break;
                case eIOtype_PONG:
                    DEBUG_WEBSOCKETS("[wsIOc] get pong\n");
                    break;
                case eIOtype_MESSAGE: {
                    if(length < 2) {
                        break;
                    }
                    socketIOmessageType_t ioType = (socketIOmessageType_t)payload[1];
                    uint8_t * data               = &payload[2];
                    size_t lData                 = length - 2;
                    switch(ioType) {
                        case sIOtype_EVENT:
                            DEBUG_WEBSOCKETS("[wsIOc] get event (%d): %s\n", lData, data);
                            break;
                        case sIOtype_CONNECT:
                        case sIOtype_DISCONNECT:
                        case sIOtype_ACK:
                        case sIOtype_ERROR:
                        case sIOtype_BINARY_EVENT:
                        case sIOtype_BINARY_ACK:
                        default:
                            DEBUG_WEBSOCKETS("[wsIOc] Socket.IO Message Type %c (%02X) is not implemented\n", ioType, ioType);
                            DEBUG_WEBSOCKETS("[wsIOc] get text: %s\n", payload);
                            break;
                    }

                } break;
                case eIOtype_OPEN:
                case eIOtype_CLOSE:
                case eIOtype_UPGRADE:
                case eIOtype_NOOP:
                default:
                    DEBUG_WEBSOCKETS("[wsIOc] Engine.IO Message Type %c (%02X) is not implemented\n", eType, eType);
                    DEBUG_WEBSOCKETS("[wsIOc] get text: %s\n", payload);
                    break;
            }

            // send message to server
            // webSocket.sendTXT("message here");
        } break;
        case WStype_BIN:
            DEBUG_WEBSOCKETS("[wsIOc] get binary length: %u\n", length);
            // hexdump(payload, length);

            // send data to server
            // webSocket.sendBIN(payload, length);
            break;
    }
}
