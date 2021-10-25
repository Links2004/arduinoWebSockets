"""Minimal example of Python websocket server
handling OTA updates for ESP32 amd ESP8266

Check and upload of firmware works.
Register and state function are jus for example.
"""
# pylint: disable=W0703,E1101
import asyncio
import copy
import json
import logging
import subprocess
import threading
import time
from os import listdir
from os.path import join as join_pth
from pathlib import Path

import websockets
from packaging import version

# Logger settings
logging.basicConfig(filename="ws_server.log")
Logger = logging.getLogger('WS-OTA')
Logger.addHandler(logging.StreamHandler())
Logger.setLevel(logging.INFO)

# Path to directory with FW
fw_path = join_pth(Path().absolute(), "firmware")


def create_path(path: str) -> None:
    """Check if path exist or create it"""
    Path(path).mkdir(parents=True, exist_ok=True)


def shell(command):
    """Handle execution of shell commands"""
    with subprocess.Popen(command, shell=True,
                          stdout=subprocess.PIPE,
                          universal_newlines=True
                          ) as process:
        for stdout_line in iter(process.stdout.readline, ""):
            Logger.debug(stdout_line)
        process.stdout.close()
        return_code = process.wait()
        Logger.debug("Shell returned: %s", return_code)

        return process.returncode
    return None


async def binary_send(websocket, fw_file):
    """Read firmware file, divide it to chunks and send them"""
    with open(fw_file, "rb") as binaryfile:

        while True:
            chunk = binaryfile.read(2048)
            if not chunk:
                break
            try:
                await websocket.send(chunk)
            except Exception as exception:
                Logger.exception(exception)
                return False
            time.sleep(0.2)


def version_checker(name, vdev, vapp):
    """Parse and compare FW version"""

    if version.parse(vdev) < version.parse(vapp):
        Logger.info("Client(%s) version %s is smaller than %s: Go for update", name, vdev, vapp)
        return True
    Logger.info("Client(%s) version %s is greater or equal to %s: Not updating", name, vdev, vapp)
    return False


class WsOtaHandler (threading.Thread):
    """Thread handling ota update

    Runing ota directly from message would kill WS
    as message bus would timeout.
    """
    def __init__(self, name, message, websocket):
        threading.Thread.__init__(self, daemon=True)
        self.name = name
        self.msg = message
        self.websocket = websocket

    def run(self, ):
        try:
            asyncio.run(self.start_)
        except Exception as exception:
            Logger.exception(exception)
        finally:
            pass

    async def start_(self):
        """Start _ota se asyncio future"""
        msg_task = asyncio.ensure_future(
            self._ota())

        done, pending = await asyncio.wait(
            [msg_task],
            return_when=asyncio.FIRST_COMPLETED,
        )
        Logger.info("WS Ota Handler done: %s", done)
        for task in pending:
            task.cancel()

    async def _ota(self):
        """Check for new fw and update or pass"""
        device_name = self.msg['name']
        device_chip = self.msg['chip']
        device_version = self.msg['version']
        fw_version = ''
        fw_name = ''
        fw_device = ''

        for filename in listdir(fw_path):
            fw_info = filename.split("-")
            fw_device = fw_info[0]
            if fw_device == device_name:
                fw_version = fw_info[1]
                fw_name = filename
                break

        if not fw_version:
            Logger.info("Client(%s): No fw found!", device_name)
            msg = '{"type": "ota", "value":"ok"}'
            await self.websocket.send(msg)
            return

        if not version_checker(device_name, device_version, fw_version):
            return

        fw_file = join_pth(fw_path, fw_name)
        if device_chip == 'esp8266' and not fw_file.endswith('.gz'):
            # We can compress fw to make it smaller for upload
            fw_cpress = fw_file
            fw_file = fw_cpress + ".gz"
            cpress = f"gzip -9 {fw_cpress}"
            cstate = shell(cpress)
            if cstate:
                Logger.error("Cannot compress firmware: %s", fw_name)
                return

        # Get size of fw
        size = Path(fw_file).stat().st_size

        # Request ota mode
        msg = '{"type": "ota", "value":"go", "size":' + str(size) + '}'
        await self.websocket.send(msg)

        # send file by chunks trough websocket
        await binary_send(self.websocket, fw_file)


async def _register(websocket, message):
    mac = message.get('mac')
    name = message.get('name')
    Logger.info("Client(%s) mac: %s", name, mac)
    # Some code

    response = {'response_type': 'registry', 'state': 'ok'}
    await websocket.send(json.dumps(response))


async def _state(websocket, message):
    mac = message.get('mac')
    name = message.get('name')
    Logger.info("Client(%s) mac: %s", name, mac)
    # Some code

    response = {'response_type': 'state', 'state': 'ok'}
    await websocket.send(json.dumps(response))


async def _unhandleld(websocket, msg):
    Logger.info("Unhandled message from device: %s", str(msg))
    response = {'response_type': 'response', 'state': 'nok'}
    await websocket.send(json.dumps(response))


async def _greetings(websocket, message):
    WsOtaHandler('thread_ota', copy.deepcopy(message), websocket).start()


async def message_received(websocket, message) -> None:
    """Handle incoming messages

    Check if message contain json and run waned function
    """
    switcher = {"greetings": _greetings,
                "register":  _register,
                "state":     _state
                }

    if message[0:1] == "{":
        try:
            msg_json = json.loads(message)
        except Exception as exception:
            Logger.error(exception)
            return

        type_ = msg_json.get('type')
        name = msg_json.get('name')
        func = switcher.get(type_, _unhandleld)
        Logger.debug("Client(%s)said: %s", name, type_)

        try:
            await func(websocket, msg_json)
        except Exception as exception:
            Logger.error(exception)


# pylint: disable=W0613
async def ws_server(websocket, path) -> None:
    """Run in cycle and wait for new messages"""
    async for message in websocket:
        await message_received(websocket, message)


async def main():
    """Server starter

    Normal user can bind only port nubers greater than 1024
    """
    async with websockets.serve(ws_server, "10.0.1.5", 8081):
        await asyncio.Future()  # run forever


create_path(fw_path)
asyncio.run(main())
