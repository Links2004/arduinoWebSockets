#pragma once

#include <WebSocketsNetworkClient.h>

class WebSocketsNetworkClientSecure : public WebSocketsNetworkClient {
  public:
    WebSocketsNetworkClientSecure();
    WebSocketsNetworkClientSecure(WiFiClient wifi_client);
    virtual ~WebSocketsNetworkClientSecure();

    int connect(IPAddress ip, uint16_t port) override;
    int connect(const char * host, uint16_t port) override;
    int connect(const char * host, uint16_t port, int32_t timeout) override;
    size_t write(uint8_t) override;
    size_t write(const uint8_t * buf, size_t size) override;
    size_t write(const char * str) override;
    int available() override;
    int read() override;
    int read(uint8_t * buf, size_t size) override;
    int peek() override;
    void flush() override;
    void stop() override;
    uint8_t connected() override;
    operator bool() override;

    void setCACert(const char * rootCA);
    void setCACertBundle(const uint8_t * bundle);
    void setInsecure();
    bool verify(const char * fingerprint, const char * domain_name);
};
