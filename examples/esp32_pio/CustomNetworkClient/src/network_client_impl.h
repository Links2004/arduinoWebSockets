#pragma once

#include <SSLClient.h>
#include <TinyGSM.h>
#include <WebSocketsNetworkClientSecure.h>
#include <WiFiClientSecure.h>

struct WebSocketsNetworkClient::Impl {
  static void enableWifi() {
    // Do nothing if already enabled
    if (wifi_client_ && wifi_client_secure_) {
      return;
    }
    wifi_client_ = std::unique_ptr<WiFiClient>(new WiFiClient());
    wifi_client_secure_ =
        std::unique_ptr<WiFiClientSecure>(new WiFiClientSecure());
  }
  static void disableWifi() {
    wifi_client_ = nullptr;
    wifi_client_secure_ = nullptr;
  }
  static void enableGsm(TinyGsm* tiny_gsm) {
    // Do nothing if already enabled
    if (gsm_client_ && gsm_client_secure_) {
      return;
    }
    gsm_client_ = std::unique_ptr<TinyGsmClient>(new TinyGsmClient(*tiny_gsm));
    gsm_client_secure_ =
        std::unique_ptr<SSLClient>(new SSLClient(gsm_client_.get()));
  }
  static void disableGsm() {
    if (gsm_client_secure_) {
      gsm_client_secure_->stop();
    }
    gsm_client_secure_ = nullptr;
    gsm_client_ = nullptr;
  }

  static std::unique_ptr<WiFiClient> wifi_client_;
  static std::unique_ptr<WiFiClientSecure> wifi_client_secure_;
  static std::unique_ptr<TinyGsmClient> gsm_client_;
  static std::unique_ptr<SSLClient> gsm_client_secure_;

  static const char* no_interface_error_;
};