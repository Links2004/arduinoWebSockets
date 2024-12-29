WebSocket Server and Client for Arduino [![Build Status](https://github.com/Links2004/arduinoWebSockets/actions/workflows/main.yml/badge.svg?branch=master)](https://github.com/Links2004/arduinoWebSockets/actions?query=branch%3Amaster)
===========================================

a WebSocket Server and Client for Arduino based on RFC6455.


##### Supported features of RFC6455 #####
 - text frame
 - binary frame
 - connection close
 - ping
 - pong
 - continuation frame

##### Limitations #####
 - max input length is limited to the ram size and the ```WEBSOCKETS_MAX_DATA_SIZE``` define
 - max output length has no limit (the hardware is the limit)
 - Client send big frames with mask 0x00000000 (on AVR all frames)
 - continuation frame reassembly need to be handled in the application code

 ##### Limitations for Async #####
 - Functions called from within the context of the websocket event might not honor `yield()` and/or `delay()`.  See [this issue](https://github.com/Links2004/arduinoWebSockets/issues/58#issuecomment-192376395) for more info and a potential workaround.
 - wss / SSL is not possible.

##### Supported Hardware #####
 - ESP8266 [Arduino for ESP8266](https://github.com/esp8266/Arduino/)
 - ESP32 [Arduino for ESP32](https://github.com/espressif/arduino-esp32)
 - ESP31B
 - Raspberry Pi Pico W [Arduino for Pico](https://github.com/earlephilhower/arduino-pico)
 - Particle with STM32 ARM Cortex M3
 - ATmega328 with Ethernet Shield (ATmega branch)
 - ATmega328 with enc28j60 (ATmega branch)
 - ATmega2560 with Ethernet Shield (ATmega branch)
 - ATmega2560 with enc28j60 (ATmega branch)
 - Arduino UNO [R4 WiFi](https://github.com/arduino/ArduinoCore-renesas)
 - Arduino Nano 33 IoT, MKR WIFI 1010 (requires [WiFiNINA](https://github.com/arduino-libraries/WiFiNINA/) library)
 - Seeeduino XIAO, Seeeduino Wio Terminal (requires [rpcWiFi](https://github.com/Seeed-Studio/Seeed_Arduino_rpcWiFi) library)

###### Note: ######

  version 2.0.0 and up is not compatible with AVR/ATmega, check ATmega branch.

  version 2.3.0 has API changes for the ESP8266 BareSSL (may brakes existing code)

  Arduino for AVR not supports std namespace of c++.

### wss / SSL ###
 supported for:
 - wss client on the ESP8266
 - wss / SSL for ESP32 in client mode
 - wss / SSL is not natively supported in WebSocketsServer however it is possible to achieve secure websockets
   by running the device behind an SSL proxy. See [Nginx](examples/Nginx/esp8266.ssl.reverse.proxy.conf) for a
   sample Nginx server configuration file to enable this.

### Root CA Cert Bundles for SSL/TLS connections ###

Secure connections require the certificate of the server to be verified. One option is to provide a single certificate in the chain of trust. However, for flexibility and robustness, a certificate bundle is recommended. If a server changes the root CA from which it derives its certificates, this will not be a problem. With a single CA cert it will not connect.

 - For [technical details](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/esp_crt_bundle.html)
 - For a [PlatformIO setup](https://github.com/Duckle29/esp32-certBundle/)
 - For an [example](examples/esp32/WebSocketClientSSLBundle/)

Including a bundle with all CA certs will use 77.2 kB but this list can be reduced to 16.5 kB for the 41 most common. This results in 90% absolute usage coverage and 99% market share coverage according to [W3Techs](https://w3techs.com/technologies/overview/ssl_certificate). The bundle is inserted into the compiled firmware. The bundle is not loaded into RAM, only its index.

### ESP Async TCP ###

This libary can run in Async TCP mode on the ESP.

The mode can be activated in the ```WebSockets.h``` (see WEBSOCKETS_NETWORK_TYPE define).

[ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP) libary is required.


### High Level Client API ###

 - `begin` : Initiate connection sequence to the websocket host.
```c++
void begin(const char *host, uint16_t port, const char * url = "/", const char * protocol = "arduino");
void begin(String host, uint16_t port, String url = "/", String protocol = "arduino");
```
 - `onEvent`: Callback to handle for websocket events

```c++
 void onEvent(WebSocketClientEvent cbEvent);
```

 - `WebSocketClientEvent`: Handler for websocket events
```c++
 void (*WebSocketClientEvent)(WStype_t type, uint8_t * payload, size_t length)
```
Where `WStype_t type` is defined as:
```c++
  typedef enum {
      WStype_ERROR,
      WStype_DISCONNECTED,
      WStype_CONNECTED,
      WStype_TEXT,
      WStype_BIN,
      WStype_FRAGMENT_TEXT_START,
      WStype_FRAGMENT_BIN_START,
      WStype_FRAGMENT,
      WStype_FRAGMENT_FIN,
      WStype_PING,
      WStype_PONG,
  } WStype_t;
```

### Issues ###
Submit issues to: https://github.com/Links2004/arduinoWebSockets/issues

### License and credits ###

The library is licensed under [LGPLv2.1](https://github.com/Links2004/arduinoWebSockets/blob/master/LICENSE)

[libb64](http://libb64.sourceforge.net/) written by Chris Venter. It is distributed under Public Domain see [LICENSE](https://github.com/Links2004/arduinoWebSockets/blob/master/src/libb64/LICENSE).
