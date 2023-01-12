#!/usr/bin/env python3

# python websocket client to test with
# emulator: server is at ws://127.0.0.1:9080/ws
# esp8266:  server is at ws:///ws
# (uncomment the right line below)

#uri = "ws://127.0.0.1:9080/ws"
uri = "ws://arduinoWebsockets.local/ws"

import websocket
try:
    import thread
except ImportError:
    import _thread as thread
import time

def on_message(ws, message):
    print("message");
    print(message)

def on_error(ws, error):
    print("error")
    print(error)

def on_close(ws):
    print("### closed ###")

def on_open(ws):
    print("opened")
    def run(*args):
        for i in range(3):
            time.sleep(1)
            ws.send("Hello %d" % i)
        time.sleep(1)
        ws.close()
        print("thread terminating...")
    thread.start_new_thread(run, ())


if __name__ == "__main__":
    websocket.enableTrace(True)
    ws = websocket.WebSocketApp(uri, on_message = on_message, on_error = on_error, on_close = on_close)
    ws.on_open = on_open
    ws.run_forever()
