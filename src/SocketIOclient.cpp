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
    WebSocketsClient::begin(host, port, url, protocol);
    _client.isSocketIO = true;
}

void SocketIOclient::begin(String host, uint16_t port, String url, String protocol) {
    SocketIOclient::begin(host, port, url, protocol);
}

#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266) || (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP32)
void SocketIOclient::beginSSL(const char *host, uint16_t port, const char * url, const char * fingerprint, const char * protocol) {
    WebSocketsClient::beginSSL(host, port, url, fingerprint, protocol);
    _client.isSocketIO = true;
}

void SocketIOclient::beginSSL(String host, uint16_t port, String url, String fingerprint, String protocol) {
    WebSocketsClient::beginSSL(host, port, url, fingerprint, protocol);
}
#endif

bool SocketIOclient::isConnected(void) {
    return WebSocketsClient::isConnected();
}

void SocketIOclient::disconnect(void) {
    WebSocketsClient::disconnect();
}

/**
 * send text data to client
 * @param num uint8_t client id
 * @param payload uint8_t *
 * @param length size_t
 * @param headerToPayload bool  (see sendFrame for more details)
 * @return true if ok
 */
bool SocketIOclient::sendMESSAGE(socketIOmessageType_t type, uint8_t * payload, size_t length, bool headerToPayload) {
    bool ret = false;
    if(length == 0) {
        length = strlen((const char *) payload);
    }
    if(clientIsConnected(&_client)) {

        if(!headerToPayload) {
            // webSocket Header
            ret = WebSocketsClient::sendFrameHeader(&_client, WSop_text, length + 2, true);
            // Engine.IO / Socket.IO Header
            if(ret) {
                uint8_t buf[3] = { eIOtype_MESSAGE, type, 0x00 };
                ret = WebSocketsClient::write(&_client, buf, 2);
            }
            if(ret) {
                ret = WebSocketsClient::write(&_client, payload, length );
            }
            return ret;
        } else {
            // TODO implement
        }

        // return WebSocketsClient::sendFrame(&_client, WSop_text, payload, length, true, true, headerToPayload);
    }
    return false;
}

void SocketIOclient::loop(void) {
    WebSocketsClient::loop();
    unsigned long t = millis();
    if((t - _lastConnectionFail) > EIO_HEARTBEAT_INTERVAL) {
        _lastConnectionFail = t;
        WebSocketsClient::sendTXT(eIOtype_PING);
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
                    WebSocketsClient::sendTXT(payload, length, false);
                    break;
                case eIOtype_PONG:
                    DEBUG_WEBSOCKETS("[wsIOc] get pong\n");
                    break;
                case eIOtype_MESSAGE: {
                    if(length < 2) {
                        break;
                    }
                    socketIOmessageType_t ioType = (socketIOmessageType_t) payload[1];
                    uint8_t * data = &payload[2];
                    size_t lData = length - 2;
                    socketIOPacket_t packet;
                    switch(ioType) {
                        case sIOtype_EVENT:
                            DEBUG_WEBSOCKETS("[wsIOc] get event (%d): %s\n", lData, data);
                            packet = parse(std::string((char *)data));
                            triggerEvent(packet);
                            break;
                        case sIOtype_CONNECT:
                            DEBUG_WEBSOCKETS("[wsIOc] connected\n");
                            packet.event = "connect";
                            triggerEvent(packet);
                            break;
                        case sIOtype_DISCONNECT:
                            DEBUG_WEBSOCKETS("[wsIOc] disconnected\n");
                            packet.event = "disconnect";
                            triggerEvent(packet);
                            break;
                        case sIOtype_ACK:
                        case sIOtype_ERROR:
                        case sIOtype_BINARY_EVENT:
                        case sIOtype_BINARY_ACK:
                        default:
                            DEBUG_WEBSOCKETS("[wsIOc] Socket.IO Message Type %c (%02X) is not implemented\n", ioType, ioType);
                            DEBUG_WEBSOCKETS("[wsIOc] get text: %s\n", payload);
                            break;
                    }

                }
                    break;
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

void SocketIOclient::triggerEvent(const socketIOPacket_t &packet) {
    ackCallback_fn b = [this, packet](const char *cb_payload) {
        String msg = constructMsg(packet.event.c_str(), cb_payload, packet.id.c_str());
        sendMESSAGE(sIOtype_ACK, (uint8_t *)msg.c_str(), msg.length(), false);
    };
    auto e = _events.find(packet.event.c_str());
    if (e != _events.end()) {
        DEBUG_WEBSOCKETS("[wsIOc] trigger event %s\n", packet.event.c_str());
        e->second(packet.data, b);
    } else {
        DEBUG_WEBSOCKETS("[wsIOc] event %s not found. %d events available\n", packet.event.c_str(), _events.size());
    }
}

void SocketIOclient::emit(const char *event, const char *payload) {
    String msg = constructMsg(event, payload);
    DEBUG_WEBSOCKETS("[wsIOc] emit %s\n", msg.c_str());
    sendMESSAGE(sIOtype_EVENT, (uint8_t *)msg.c_str(), msg.length(), false);
}

String SocketIOclient::constructMsg(const char *event, const char *payload, const char *id) {
    String msg = String("");
    if (id) {
        msg += id;
    }
    msg += "[\"";
    msg += event;
    msg += "\"";
    if (payload) {
        msg += ",";
        if (payload[0] != '{' && payload[0] != '[')
            msg += "\"";
        msg += payload;
        if (payload[0] != '{' && payload[0] != '[')
            msg += "\"";
    }
    msg += "]";
    return msg;
}

void SocketIOclient::on(const char *event, callback_fn func) {
    _events[event] = func;
}

socketIOPacket_t SocketIOclient::parse(const std::string &payloadStr) {
    socketIOPacket_t result;
    unsigned int currentParseType = eParseTypeID;
    bool escChar = false;
    bool inString = false;
    bool inJson = false;
    bool inArray = false;
    for (auto c : payloadStr) {
        if (!escChar && c == '"') {
            if (inString)
                inString = false;
            else
                inString = true;
            if (!inJson && !inArray)
                continue;
        }
        if (c == '\\') {
            if (escChar)
                escChar = false;
            else
                escChar = true;
        } else if (escChar)
            escChar = false;

        if (!inJson && !inString && c == '{')
            inJson = true;
        if (inJson && !inString && c == '}')
            inJson = false;

        if (currentParseType > eParseTypeID)
            if (!inArray && !inString && c == '[')
                inArray = true;

        if (!inArray && !inString && !inJson && !escChar) {
            if (c == '[' && currentParseType == eParseTypeID) {
                currentParseType++;
                continue;
            }
            if (c == ',' && currentParseType == eParseTypeEVENT) {
                currentParseType++;
                continue;
            }
            if (c == ']') {
                break;
            }
        }

        if (currentParseType > eParseTypeID)
            if (inArray && !inString && c == ']')
                inArray = false;

        if (currentParseType == eParseTypeID) {
            result.id += c;
        } else if (currentParseType == eParseTypeEVENT) {
            result.event += c;
        } else if (currentParseType == eParseTypeDATA) {
            result.data += c;
        }
    }
    return result;
}
