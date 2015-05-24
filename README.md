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
 - Client send masked frames always with mask 0x00000000 (open todo)

##### Supported Hardware #####
 - ESP8266 [Arduino for ESP8266](https://github.com/Links2004/Arduino)
 - ATmega328 with Ethernet Shield (planed)
 - ATmega328 with enc28j60 (planed)
 - ATmega2560 with Ethernet Shield (planed)
 - ATmega2560 with enc28j60 (planed)
 
### Issues ###
Submit issues to: https://github.com/Links2004/arduinoWebSockets/issues

### License and credits ###

The library is licensed under [LGPLv2.1](https://github.com/Links2004/arduinoWebSockets/blob/master/LICENSE)

[libb64](http://libb64.sourceforge.net/) written by Chris Venter. It is distributed under Public Domain see [LICENSE](https://github.com/Links2004/arduinoWebSockets/blob/master/src/libb64/LICENSE).
