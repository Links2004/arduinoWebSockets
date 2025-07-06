#include "network_client_impl.h"

WebSocketsNetworkClientSecure::WebSocketsNetworkClientSecure() {}
WebSocketsNetworkClientSecure::WebSocketsNetworkClientSecure(
    WiFiClient wifi_client) {}
WebSocketsNetworkClientSecure::~WebSocketsNetworkClientSecure() {
  if (_impl->gsm_client_secure_) {
    _impl->gsm_client_secure_->stop();
  }
}

int WebSocketsNetworkClientSecure::connect(IPAddress ip, uint16_t port) {
  if (_impl->gsm_client_secure_) {
    return _impl->gsm_client_secure_->connect(ip, port);
  } else if (_impl->wifi_client_secure_) {
    return _impl->wifi_client_secure_->connect(ip, port);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

int WebSocketsNetworkClientSecure::connect(const char *host, uint16_t port) {
  if (_impl->gsm_client_secure_) {
    return _impl->gsm_client_secure_->connect(host, port);
  } else if (_impl->wifi_client_secure_) {
    return _impl->wifi_client_secure_->connect(host, port);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}
int WebSocketsNetworkClientSecure::connect(const char *host, uint16_t port,
                                           int32_t timeout_ms) {
  if (_impl->gsm_client_secure_) {
    // Ignore timeout as will cause read() to block for specified time
    return _impl->gsm_client_secure_->connect(host, port);
  } else if (_impl->wifi_client_secure_) {
    return _impl->wifi_client_secure_->connect(host, port, timeout_ms);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

size_t WebSocketsNetworkClientSecure::write(uint8_t data) {
  if (_impl->gsm_client_secure_) {
    return _impl->gsm_client_secure_->write(data);
  } else if (_impl->wifi_client_secure_) {
    return _impl->wifi_client_secure_->write(data);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

size_t WebSocketsNetworkClientSecure::write(const uint8_t *buf, size_t size) {
  if (_impl->gsm_client_secure_) {
    return _impl->gsm_client_secure_->write(buf, size);
  } else if (_impl->wifi_client_secure_) {
    return _impl->wifi_client_secure_->write(buf, size);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

size_t WebSocketsNetworkClientSecure::write(const char *str) {
  const int size = strlen(str);
  if (_impl->gsm_client_secure_) {
    return _impl->gsm_client_secure_->write((const uint8_t *)str, size);
  } else if (_impl->wifi_client_secure_) {
    return _impl->wifi_client_secure_->write((const uint8_t *)str, size);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

int WebSocketsNetworkClientSecure::available() {
  if (_impl->gsm_client_secure_) {
    return _impl->gsm_client_secure_->available();
  } else if (_impl->wifi_client_secure_) {
    return _impl->wifi_client_secure_->available();
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

int WebSocketsNetworkClientSecure::read() {
  if (_impl->gsm_client_secure_) {
    return _impl->gsm_client_secure_->read();
  } else if (_impl->wifi_client_secure_) {
    return _impl->wifi_client_secure_->read();
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

int WebSocketsNetworkClientSecure::read(uint8_t *buf, size_t size) {
  if (_impl->gsm_client_secure_) {
    return _impl->gsm_client_secure_->read(buf, size);
  } else if (_impl->wifi_client_secure_) {
    return _impl->wifi_client_secure_->read(buf, size);
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

int WebSocketsNetworkClientSecure::peek() {
  if (_impl->gsm_client_secure_) {
    return _impl->gsm_client_secure_->peek();
  } else if (_impl->wifi_client_secure_) {
    return _impl->wifi_client_secure_->peek();
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

void WebSocketsNetworkClientSecure::flush() {
  if (_impl->gsm_client_secure_) {
    return _impl->gsm_client_secure_->flush();
  } else if (_impl->wifi_client_secure_) {
    return _impl->wifi_client_secure_->flush();
  }
  Serial.println(_impl->no_interface_error_);
}

void WebSocketsNetworkClientSecure::stop() {
  if (_impl->gsm_client_secure_) {
    return _impl->gsm_client_secure_->stop();
  } else if (_impl->wifi_client_secure_) {
    return _impl->wifi_client_secure_->stop();
  }
  Serial.println(_impl->no_interface_error_);
}

uint8_t WebSocketsNetworkClientSecure::connected() {
  if (_impl->gsm_client_secure_) {
    return _impl->gsm_client_secure_->connected();
  } else if (_impl->wifi_client_secure_) {
    return _impl->wifi_client_secure_->connected();
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

WebSocketsNetworkClientSecure::operator bool() {
  if (_impl->gsm_client_secure_) {
    return _impl->gsm_client_secure_->operator bool();
  } else if (_impl->wifi_client_secure_) {
    return _impl->wifi_client_secure_->operator bool();
  }
  Serial.println(_impl->no_interface_error_);
  return 0;
}

void WebSocketsNetworkClientSecure::setCACert(const char *rootCA) {
  if (_impl->gsm_client_secure_) {
    return _impl->gsm_client_secure_->setCertificate(rootCA);
  } else if (_impl->wifi_client_secure_) {
    return _impl->wifi_client_secure_->setCACert(rootCA);
  }
  Serial.println(_impl->no_interface_error_);
}

void WebSocketsNetworkClientSecure::setCACertBundle(const uint8_t *bundle) {
  if (_impl->gsm_client_secure_) {
    return _impl->gsm_client_secure_->setCACertBundle(bundle);
  } else if (_impl->wifi_client_secure_) {
    return _impl->wifi_client_secure_->setCACertBundle(bundle);
  }
  Serial.println(_impl->no_interface_error_);
}

void WebSocketsNetworkClientSecure::setInsecure() {
  if (_impl->gsm_client_secure_) {
    _impl->gsm_client_secure_->setInsecure();
  } else if (_impl->wifi_client_secure_) {
    _impl->wifi_client_secure_->setInsecure();
  }
  Serial.println(_impl->no_interface_error_);
}

bool WebSocketsNetworkClientSecure::verify(const char *fingerprint,
                                           const char *domain_name) {
  if (_impl->gsm_client_secure_) {
    // Simply calling SSLClient::verify() will break TLS handshake
    // Can be skipped as verification is done by SSLClient itself,
    // ArduinoWebSockets need not call it
    return true;
  } else if (_impl->wifi_client_secure_) {
    return _impl->wifi_client_secure_->verify(fingerprint, domain_name);
  }
  Serial.println(_impl->no_interface_error_);
  return false;
}
