#include "network_client_impl.h"

WebSocketsNetworkClient::WebSocketsNetworkClient()
    : _impl(new WebSocketsNetworkClient::Impl()) {}
WebSocketsNetworkClient::WebSocketsNetworkClient(WiFiClient wifi_client)
    : _impl(new WebSocketsNetworkClient::Impl()) {}
WebSocketsNetworkClient::~WebSocketsNetworkClient() {}

int WebSocketsNetworkClient::connect(IPAddress ip, uint16_t port) {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->connect(ip, port);
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->connect(ip, port);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

int WebSocketsNetworkClient::connect(const char *host, uint16_t port) {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->connect(host, port);
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->connect(host, port);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}
int WebSocketsNetworkClient::connect(const char *host, uint16_t port,
                                     int32_t timeout_ms) {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->connect(host, port, timeout_ms);
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->connect(host, port, timeout_ms);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

size_t WebSocketsNetworkClient::write(uint8_t data) {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->write(data);
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->write(data);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

size_t WebSocketsNetworkClient::write(const uint8_t *buf, size_t size) {
  Serial.printf("Send_: %zu\n", size);
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->write(buf, size);
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->write(buf, size);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

size_t WebSocketsNetworkClient::write(const char *str) {
  const int size = strlen(str);
  Serial.printf("Send: %zu\n", size);
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->write((const uint8_t *)str, size);
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->write((const uint8_t *)str, size);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

int WebSocketsNetworkClient::available() {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->available();
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->available();
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

int WebSocketsNetworkClient::read() {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->read();
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->read();
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

int WebSocketsNetworkClient::read(uint8_t *buf, size_t size) {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->read(buf, size);
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->read(buf, size);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

int WebSocketsNetworkClient::peek() {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->peek();
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->peek();
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

void WebSocketsNetworkClient::flush() {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->flush();
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->flush();
  }
  Serial.println(_impl->no_interface_error_);
}

void WebSocketsNetworkClient::stop() {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->stop();
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->stop();
  }
  Serial.println(_impl->no_interface_error_);
}

uint8_t WebSocketsNetworkClient::connected() {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->connected();
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->connected();
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

WebSocketsNetworkClient::operator bool() {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->operator bool();
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->operator bool();
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}
