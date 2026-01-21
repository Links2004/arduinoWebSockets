#pragma once

#include <Client.h>
#include <WiFiClient.h>

class WebSocketsNetworkClient : public Client {
  public:
    struct Impl;
    std::unique_ptr<Impl> _impl;

    WebSocketsNetworkClient();
    WebSocketsNetworkClient(WiFiClient wifi_client);
    virtual ~WebSocketsNetworkClient();

    virtual int connect(IPAddress ip, uint16_t port);
    virtual int connect(const char * host, uint16_t port);
    virtual int connect(const char * host, uint16_t port, int32_t timeout);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t * buf, size_t size);
    virtual size_t write(const char * str);
    virtual int available();
    virtual int read();
    virtual int read(uint8_t * buf, size_t size);
    virtual int peek();
    virtual void flush();
    virtual void stop();
    virtual uint8_t connected();
    virtual operator bool();
};
