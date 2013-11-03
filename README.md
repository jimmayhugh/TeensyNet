TeensyNet is a networked 1-Wire Controller.

The main board consists of a Teensy 3.0 Cortex ARM mini-board, a choice of WIZ812mj, WIZ820io, or CC3000 mini nics, and 3v3 to 5v0 buffers. Each TeensyNet can control up to 12 discrete 1-wire devices, currently DS18B20 digital thermometers and DS2406+ digital switches.

There are up to 4 "Actions", that can consist of 1 DS18B20 and 2 DS2406+ devices. The DS2406 devices can be turned on or off based on the value returned by the DS18B20. A timed delay may also be set for each switch action.

There are also 4 available PID controls that can be used with 1 DS18B20 and 1 DS2406+.

The TeensyNet is controlled and monitored by means of UDP over either a wired or wireless internet/intranet connection.
The TeensyNet uses DHCP to obtain an IP address, and a series of PHP-based web pages are used to monitor and control the TeensyNet.

This system was designed to be used on an local intranet, and is NOT secure if exposed to the outside world. Connect it to the internet at your own risk.

This repository contains both the TeensyNet*.ino code and the web interface files.
