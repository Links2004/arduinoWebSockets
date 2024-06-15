#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>
#include "WiFiClientSecure.h"

#include "secrets.hpp"

#define SSL_TEST_URL "https://raw.githubusercontent.com/Duckle29/esp32-certBundle/main/readme.md"

extern const uint8_t x509_crt_bundle_start[] asm("_binary_src_x509_crt_bundle_start");
extern const uint8_t x509_crt_bundle_end[] asm("_binary_src_x509_crt_bundle_end");

WiFiMulti WiFiMulti;

void setClock(void);
