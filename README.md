WebSocket Server and Client for Arduino
===========================================

a WebSocket Server and Client for Arduino based on RFC6455.

 
##### Supported features of RFC6455 #####
 - text frame
 - binary frame
 - connection close
 - ping
 - pong
 
##### Not supported features of RFC6455 #####
 - continuation frame
  
##### Limitations #####
 - max input length is limited to the ram size and the ```WEBSOCKETS_MAX_DATA_SIZE``` define
 - max output length has no limit (the hardware is the limit)
 - Client send masked frames always with mask 0x00000000

##### Supported Hardware #####
 - ESP8266 [Arduino for ESP8266](https://github.com/Links2004/Arduino)
 - ATmega328 with Ethernet Shield (alpha)
 - ATmega328 with enc28j60 (alpha)
 - ATmega2560 with Ethernet Shield (alpha)
 - ATmega2560 with enc28j60 (alpha)
 
### wss / SSL ###
 supported for:
 - wss client on the ESP8266
 
### Issues ###
Submit issues to: https://github.com/Links2004/arduinoWebSockets/issues

[![Join the chat at https://gitter.im/Links2004/arduinoWebSockets](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/Links2004/arduinoWebSockets?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

### License and credits ###

The library is licensed under [LGPLv2.1](https://github.com/Links2004/arduinoWebSockets/blob/master/LICENSE)

[libb64](http://libb64.sourceforge.net/) written by Chris Venter. It is distributed under Public Domain see [LICENSE](https://github.com/Links2004/arduinoWebSockets/blob/master/src/libb64/LICENSE).
