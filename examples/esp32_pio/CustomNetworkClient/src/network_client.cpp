#include "network_client_impl.h"

NetworkClient::NetworkClient() : _impl(new NetworkClient::Impl()) {}
NetworkClient::NetworkClient(WiFiClient wifi_client)
    : _impl(new NetworkClient::Impl()) {}
NetworkClient::~NetworkClient() {}

int NetworkClient::connect(IPAddress ip, uint16_t port) {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->connect(ip, port);
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->connect(ip, port);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

int NetworkClient::connect(const char *host, uint16_t port) {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->connect(host, port);
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->connect(host, port);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}
int NetworkClient::connect(const char *host, uint16_t port,
                           int32_t timeout_ms) {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->connect(host, port, timeout_ms);
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->connect(host, port, timeout_ms);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

size_t NetworkClient::write(uint8_t data) {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->write(data);
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->write(data);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

size_t NetworkClient::write(const uint8_t *buf, size_t size) {
  Serial.printf("Send_: %zu\n", size);
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->write(buf, size);
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->write(buf, size);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

size_t NetworkClient::write(const char *str) {
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

int NetworkClient::available() {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->available();
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->available();
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

int NetworkClient::read() {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->read();
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->read();
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

int NetworkClient::read(uint8_t *buf, size_t size) {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->read(buf, size);
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->read(buf, size);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

int NetworkClient::peek() {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->peek();
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->peek();
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

void NetworkClient::flush() {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->flush();
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->flush();
  }
  Serial.println(_impl->no_interface_error_);
}

void NetworkClient::stop() {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->stop();
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->stop();
  }
  Serial.println(_impl->no_interface_error_);
}

uint8_t NetworkClient::connected() {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->connected();
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->connected();
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

NetworkClient::operator bool() {
  if (_impl->gsm_client_) {
    return _impl->gsm_client_->operator bool();
  } else if (_impl->wifi_client_) {
    return _impl->wifi_client_->operator bool();
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}
