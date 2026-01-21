This example project shows how to use a custom NetworkClient class to switch between two underlying network interfaces.

In this case it shows how a board can support Wi-Fi as well as GSM/LTE connectivity. It uses a shim to switch between the two network interfaces, thereby allowing a single binary to handle both interfaces without needing to reboot. This example can be extended to cover other network interfaces that conform to the Client() class interface.
