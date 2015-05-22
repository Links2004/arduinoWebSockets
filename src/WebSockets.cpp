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
 *
 * @param client WSclient_t *  ptr to the client struct
 * @param code
 * @param reason
 * @param reasonLen
 */
void WebSockets::clientDisconnect(WSclient_t * client, uint16_t code, char * reason, size_t reasonLen) {
    if(client->status == WSC_CONNECTED && code) {
        //todo send reason to client

        if(reasonLen > 0 && reason) {

        } else {

        }
    }
    clientDisconnect(client);
}

/**
 *
 * @param client WSclient_t *  ptr to the client struct
 * @param opcode WSopcode_t
 * @param payload uint8_t *
 * @param lenght size_t
 */
void WebSockets::sendFrame(WSclient_t * client, WSopcode_t opcode, uint8_t * payload, size_t lenght) {

    uint8_t buffer[16] = { 0 };
    uint8_t i = 0;

    //create header

    buffer[i] = bit(7);     // set Fin
    buffer[i++] |= opcode;    // set opcode

    if(lenght < 126) {
        buffer[i++] = lenght;

    } else if(lenght < 0xFFFF) {
        buffer[i++] = 126;
        buffer[i++] = ((lenght >> 8) & 0xFF);
        buffer[i++] = (lenght & 0xFF);
    } else {
        buffer[i++] = 127;
        buffer[i++] = 0x00;
        buffer[i++] = 0x00;
        buffer[i++] = 0x00;
        buffer[i++] = 0x00;
        buffer[i++] = ((lenght >> 24) & 0xFF);
        buffer[i++] = ((lenght >> 16) & 0xFF);
        buffer[i++] = ((lenght >> 8) & 0xFF);
        buffer[i++] = (lenght & 0xFF);
    }

    // send header
    client->tcp.write(&buffer[0], i);

    if(payload && lenght > 0) {
        // send payload
        client->tcp.write(&payload[0], lenght);
    }

}

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
        clientDisconnect(client, 1002);
        return;
    }

    // split first 2 bytes in the data
    fin = ((buffer[0] >> 7) & 0x01);
    rsv1 = ((buffer[0] >> 6) & 0x01);
    rsv2 = ((buffer[0] >> 5) & 0x01);
    rsv3 = ((buffer[0] >> 4) & 0x01);
    opCode = (WSopcode_t) (buffer[0] & 0x0F);

    mask = ((buffer[1] >> 7) & 0x01);
    payloadLen = (WSopcode_t) (buffer[1] & 0x7F);

    if(payloadLen == 126) {
        if(!readWait(client, buffer, 2)) {
            //timeout
            clientDisconnect(client, 1002);
            return;
        }
        payloadLen = buffer[0] << 8 | buffer[1];
    } else if(payloadLen == 127) {
        // read 64bit inteager as Lenght
        if(!readWait(client, buffer, 8)) {
            //timeout
            clientDisconnect(client, 1002);
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
        clientDisconnect(client, 1009);
        return;
    }

    if(mask) {
        client->tcp.read(maskKey, 4);
    }

    if(payloadLen > 0) {
        // if text data we need one more
        payload = (uint8_t *) malloc(payloadLen + 1);

        if(!payload) {
            DEBUG_WEBSOCKETS("[WS-Server][%d][handleWebsocket] to less memory to handle payload %d!\n", client->num, payloadLen);
            clientDisconnect(client, 1011);
            return;
        }

        if(!readWait(client, payload, payloadLen)) {
            DEBUG_WEBSOCKETS("[WS-Server][%d][handleWebsocket] missing data!\n", client->num);
            clientDisconnect(client, 1002);
            return;
        }

        payload[payloadLen] = 0x00;

        if(mask) {
            //decode XOR
            for(size_t i = 0; i < payloadLen; i++) {
                payload[i] = (payload[i] ^ maskKey[i % 4]);
            }
        }

        switch(opCode) {
            case WSop_text:
                DEBUG_WEBSOCKETS("[WS-Server][%d][handleWebsocket] text: %s\n", client->num, payload)
                ;
                // todo API for user to get message may callback

                // send the frame back!
                sendFrame(client, WSop_text, payload, payloadLen);

                break;
            case WSop_binary:
                // todo API for user to get message may callback
                break;
            case WSop_ping:
                // todo send pong
                break;
            case WSop_pong:
                DEBUG_WEBSOCKETS("[WS-Server][%d][handleWebsocket] get pong from Client (%s)\n", client->num, payload)
                ;
                break;
            case WSop_close: {
                uint16_t reasonCode = buffer[0] << 8 | buffer[1];

                DEBUG_WEBSOCKETS("[WS-Server][%d][handleWebsocket] client ask for close. Code: %d (%s)\n", client->num, reasonCode, (payload + 2));

                // todo send confimation to client
                clientDisconnect(client, 1000, (char *) (payload + 2), payloadLen - 2);
            }
                break;
            case WSop_continuation:
                // continuation is not supported
                clientDisconnect(client, 1003);
                break;
            default:
                clientDisconnect(client, 1002);
                break;
        }
    }

}

/**
 * read x byte from tcp or get timeout
 * @param client WSclient_t *
 * @param out  uint8_t * data buffer
 * @param n size_t byte count
 * @return true if ok
 */
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
