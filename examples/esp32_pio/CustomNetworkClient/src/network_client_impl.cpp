#include "network_client_impl.h"

std::unique_ptr<WiFiClient> NetworkClient::Impl::wifi_client_ = nullptr;
std::unique_ptr<WiFiClientSecure> NetworkClient::Impl::wifi_client_secure_ =
    nullptr;
std::unique_ptr<TinyGsmClient> NetworkClient::Impl::gsm_client_ = nullptr;
std::unique_ptr<SSLClient> NetworkClient::Impl::gsm_client_secure_ = nullptr;
const char *NetworkClient::Impl::no_interface_error_ = "No interface";
