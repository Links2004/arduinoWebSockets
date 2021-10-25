## Minimal example of WebsocketClientOTA and Python server

Take this as small example, how achieve OTA update on ESP8266 and ESP32.

Python server was wrote from train so take it only as bare example.
It's working, but it's not mean to run in production.


### Usage:

Start server:
```bash
cd python_ota_server
python3 -m venv .venv
source .venv/bin/activate
pip3 install -r requirements.txt
python3 main.py
```

Flash ESP with example sketch and start it.

Change version inside example sketch to higher and compile it and save it to bin file.

Rename it to `mydevice-1.0.1-esp8266.bin` and place it inside new folder firmware (server create it).

When the ESP connect to server, it check if version flashed is equal to fw in firmware folder. If higher FW version is present,
start the flash process.