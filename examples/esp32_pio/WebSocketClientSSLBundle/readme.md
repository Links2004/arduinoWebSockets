This is a PlatformIO project that uses a modified WiFiClientSecure library (in `lib`) to 
implement proper SSL support using root certificates as discussed 
[here](https://github.com/espressif/arduino-esp32/issues/3646#issuecomment-648292677)

It is based on the work by [meltdonw03](https://github.com/meltdown03) in that thread, and the
[BasicHttpsClient example](https://github.com/espressif/arduino-esp32/blob/1.0.4/libraries/HTTPClient/examples/BasicHttpsClient/BasicHttpsClient.ino) from the arduino-esp32 project.

Just copy `include/secrets.hpp.template` to `include/secrets.hpp` and fill in your WiFi details.
Then it should be pretty much ready to go. The local WiFiClientSecure library should take priority.
Debug is set to verbose, so you'll see a lot of noise, but there should also be this readme on success :)

To get a current CA cert bundle download it from [curl's website](https://curl.se/docs/caextract.html).