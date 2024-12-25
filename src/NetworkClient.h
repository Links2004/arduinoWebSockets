#pragma once

#include <Client.h>
#include <WiFiClient.h>

class NetworkClient : public Client {
  public:
    NetworkClient();
    NetworkClient(WiFiClient wifi_client);
    virtual ~NetworkClient() = default;

    int connect(IPAddress ip, uint16_t port) final;
    int connect(const char * host, uint16_t port) final;
    int connect(const char * host, uint16_t port, int32_t timeout);
    size_t write(uint8_t) final;
    size_t write(const uint8_t * buf, size_t size) final;
    size_t write(const char * str);
    int available() final;
    int read() final;
    int read(uint8_t * buf, size_t size) final;
    int peek() final;
    void flush() final;
    void stop() final;
    uint8_t connected() final;
    operator bool() final;

    void setCACert(const char * rootCA);
    void setCACertBundle(const uint8_t * bundle);
    void setInsecure();
    bool verify(const char * fingerprint, const char * domain_name);
};
