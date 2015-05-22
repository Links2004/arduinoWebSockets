/**
 * @file WebSockets.cpp
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

/**
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-------+-+-------------+-------------------------------+
 |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
 |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
 |N|V|V|V|       |S|             |   (if payload len==126/127)   |
 | |1|2|3|       |K|             |                               |
 +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
 |     Extended payload length continued, if payload len == 127  |
 + - - - - - - - - - - - - - - - +-------------------------------+
 |                               |Masking-key, if MASK set to 1  |
 +-------------------------------+-------------------------------+
 | Masking-key (continued)       |          Payload Data         |
 +-------------------------------- - - - - - - - - - - - - - - - +
 :                     Payload Data continued ...                :
 + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 |                     Payload Data continued ...                |
 +---------------------------------------------------------------+
 *
 */

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



/**
 * handle the WebSocket stream
 * @param client WSclient_t *  ptr to the client struct
 */
void WebSockets::handleWebsocket(WSclient_t * client) {

    uint8_t buffer[8] = { 0 };

    bool fin;
    bool rsv1;
    bool rsv2;
    bool rsv3;
    WSopcode_t opCode;
    bool mask;
    size_t payloadLen;

    uint8_t maskKey[4];

    uint8_t * payload = NULL;

    DEBUG_WEBSOCKETS("[WS-Server][%d][handleWebsocket] ------- read massage frame -------\n", client->num);

    if(!readWait(client, buffer, 2)) {
        //timeout
        clientDisconnect(client);
        return;
    }

    // split first 2 bytes in the data
    fin = ((buffer[0] >> 7) & 0x01);
    rsv1 = ((buffer[0] >> 6) & 0x01);
    rsv2 = ((buffer[0] >> 5) & 0x01);
    rsv3 = ((buffer[0] >> 4) & 0x01);
    opCode = (WSopcode_t)(buffer[0] & 0x0F);

    mask = ((buffer[1] >> 7) & 0x01);
    payloadLen = (WSopcode_t)(buffer[1] & 0x7F);

    if(payloadLen == 126) {
        if(!readWait(client, buffer, 2)) {
            //timeout
            clientDisconnect(client);
            return;
        }
        payloadLen = buffer[0] << 8 | buffer[1];
    } else if(payloadLen == 127) {
        // read 64bit inteager as Lenght
        if(!readWait(client, buffer, 8)) {
            //timeout
            clientDisconnect(client);
            return;
        }

        if(buffer[0] != 0 || buffer[1] != 0 || buffer[2] != 0 || buffer[3] != 0) {
            // really to big!
            payloadLen = 0xFFFFFFFF;
        } else {
            payloadLen = buffer[4] << 24 | buffer[5] << 16 | buffer[6] << 8 | buffer[7];
        }
    }

    DEBUG_WEBSOCKETS("[WS-Server][%d][handleWebsocket] fin: %u rsv1: %u rsv2: %u rsv3 %u  opCode: %u\n", client->num, fin, rsv1, rsv2, rsv3, opCode);
    DEBUG_WEBSOCKETS("[WS-Server][%d][handleWebsocket] mask: %u payloadLen: %u\n", client->num, mask, payloadLen);

    if(payloadLen > WEBSOCKETS_MAX_DATA_SIZE) {
        DEBUG_WEBSOCKETS("[WS-Server][%d][handleWebsocket] payload to big! (%u)\n", client->num, payloadLen);
        clientDisconnect(client);
        return;
    }

    if(mask) {
        client->tcp.read(maskKey, 4);
    }

    if(payloadLen > 0) {
        // if text data we need one more
        payload = (uint8_t *) malloc(payloadLen+1);

        if(!payload) {
            DEBUG_WEBSOCKETS("[WS-Server][%d][handleWebsocket] to less memory to handle payload %d!\n", client->num, payloadLen);
            clientDisconnect(client);
            return;
        }

        if(!readWait(client, payload, payloadLen)) {
            DEBUG_WEBSOCKETS("[WS-Server][%d][handleWebsocket] missing data!\n", client->num);
            clientDisconnect(client);
            return;
        }

        if(mask) {
            //decode XOR
            for (size_t i = 0; i < payloadLen; i++) {
                payload[i] = (payload[i] ^ maskKey[i % 4]);
            }
        }

        if(opCode == WSop_text) {
            payload[payloadLen] = 0x00;
            DEBUG_WEBSOCKETS("[WS-Server][%d][handleWebsocket] Text: %s\n", client->num, payload);
        } else {
            clientDisconnect(client);
        }
    }

}

bool WebSockets::readWait(WSclient_t * client, uint8_t *out, size_t n) {
    unsigned long t = millis();
    size_t len;

    while(n > 0) {
        if(!client->tcp.connected()) {
            DEBUG_WEBSOCKETS("[readWait] Receive not connected - 1!\n");
            return false;
        }
        while(!client->tcp.available()) {
            if(!client->tcp.connected()) {
                DEBUG_WEBSOCKETS("[readWait] Receive not connected - 2!\n");
                return false;
            }
            if((millis() - t) > WEBSOCKETS_TCP_TIMEOUT) {
                DEBUG_WEBSOCKETS("[readWait] Receive TIMEOUT!\n");
                return false;
            }
            delay(0);
        }

        len = client->tcp.read((uint8_t*) out, n);
        if(len) {
            t = millis();
            out += len;
            n -= len;
            //DEBUG_WEBSOCKETS("Receive %d left %d!\n", len, n);
        } else {
            //DEBUG_WEBSOCKETS("Receive %d left %d!\n", len, n);
        }
        delay(0);
    }
    return true;
}
