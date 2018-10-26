/*
 * SocketIOclient.h
 *
 *  Created on: May 12, 2018
 *      Author: links
 */

#ifndef SOCKETIOCLIENT_H_
#define SOCKETIOCLIENT_H_

#include "WebSocketsClient.h"
#include <map>

#define EIO_HEARTBEAT_INTERVAL 10000

#define EIO_MAX_HEADER_SIZE  (WEBSOCKETS_MAX_HEADER_SIZE + 1)
#define SIO_MAX_HEADER_SIZE  (EIO_MAX_HEADER_SIZE + 1)

typedef enum {
    eIOtype_OPEN = '0', ///< Sent from the server when a new transport is opened (recheck)
    eIOtype_CLOSE = '1', ///< Request the close of this transport but does not shutdown the connection itself.
    eIOtype_PING = '2', ///< Sent by the client. Server should answer with a pong packet containing the same data
    eIOtype_PONG = '3', ///< Sent by the server to respond to ping packets.
    eIOtype_MESSAGE = '4', ///< actual message, client and server should call their callbacks with the data
    eIOtype_UPGRADE = '5', ///< Before engine.io switches a transport, it tests, if server and client can communicate over this transport. If this test succeed, the client sends an upgrade packets which requests the server to flush its cache on the old transport and switch to the new transport.
    eIOtype_NOOP = '6', ///< A noop packet. Used primarily to force a poll cycle when an incoming websocket connection is received.
} engineIOmessageType_t;


typedef enum {
    sIOtype_CONNECT = '0',
    sIOtype_DISCONNECT = '1',
    sIOtype_EVENT = '2',
    sIOtype_ACK = '3',
    sIOtype_ERROR = '4',
    sIOtype_BINARY_EVENT = '5',
    sIOtype_BINARY_ACK = '6',
} socketIOmessageType_t;


struct socketIOPacket_t {
    String id = "";
    String event = "";
    String data = "";
};

const unsigned int eParseTypeID = 0;
const unsigned int eParseTypeEVENT = 1;
const unsigned int eParseTypeDATA = 2;

typedef std::function<void (const char * payload)> ackCallback_fn;
typedef std::function<void (const String &payload, ackCallback_fn)> callback_fn;

class SocketIOclient: protected WebSocketsClient {

    public:
#ifdef __AVR__
        typedef void (*SocketIOclientEvent)(WStype_t type, uint8_t * payload, size_t length);
#else
        typedef std::function<void (WStype_t type, uint8_t * payload, size_t length)> SocketIOclientEvent;
#endif

        SocketIOclient(void);
        virtual ~SocketIOclient(void);

        void begin(const char *host, uint16_t port, const char * url = "/socket.io/?EIO=3", const char * protocol = "arduino");
        void begin(String host, uint16_t port, String url = "/socket.io/?EIO=3", String protocol = "arduino");

        bool isConnected(void);

        bool sendMESSAGE(socketIOmessageType_t type, uint8_t * payload, size_t length = 0, bool headerToPayload = false);
        bool sendEVENT(uint8_t * payload, size_t length = 0, bool headerToPayload = false);
        bool sendEVENT(const uint8_t * payload, size_t length = 0);
        bool sendEVENT(char * payload, size_t length = 0, bool headerToPayload = false);
        bool sendEVENT(const char * payload, size_t length = 0);
        bool sendEVENT(String & payload);

        void emit(const char* event, const char * payload = NULL);
        void on(const char* event, callback_fn);

        void loop(void);

    protected:
        void runCbEvent(WStype_t type, uint8_t * payload, size_t length);
        uint64_t _lastHeartbeat = 0;

    private:
        std::map<String, callback_fn> _events;
        void triggerEvent(const socketIOPacket_t &packet);
        String constructMsg(const char* event, const char* payload = NULL, const char * id = NULL);

        /**
         * Parses the payload into a socketIOPacket_t.
         * The payload has the following format: ID[EVENT,DATA]
         * socketIOPacket_t contains the id, event and data.
         * @param payload std::string
         */
        socketIOPacket_t parse(const std::string &payload);
};

#endif /* SOCKETIOCLIENT_H_ */
