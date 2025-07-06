#include "network_client_impl.h"

std::unique_ptr<WiFiClient> WebSocketsNetworkClient::Impl::wifi_client_ = nullptr;
std::unique_ptr<WiFiClientSecure> WebSocketsNetworkClient::Impl::wifi_client_secure_ =
    nullptr;
std::unique_ptr<TinyGsmClient> WebSocketsNetworkClient::Impl::gsm_client_ = nullptr;
std::unique_ptr<SSLClient> WebSocketsNetworkClient::Impl::gsm_client_secure_ = nullptr;
const char *WebSocketsNetworkClient::Impl::no_interface_error_ = "No interface";
