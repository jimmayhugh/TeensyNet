TeensyNet is a networked 1-Wire Controller.

The main board consists of a Teensy3.1 Cortex ARM mini-board, a WIZ820io mini nic, and 3v3 to 5v level converters. Each TeensyNet can control multiple discrete 1-wire devices, including DS18B20 digital thermometers, MAX31850 K-Type Thermocouple monitors, graphical LCDs, character-based LCDs and DS2406+ digital switches.

There are multiple "Actions", that can consist of a 1-wire thermometer or thermocouple and 2 DS2406+ devices. The DS2406 devices can be turned on or off based on the value returned by the temperature-sensing device. A timed delay may also be set for each switch action.

There are also available PID controls that can be used with 1 DS18B20 and 1 DS2406+.

The TeensyNet is controlled and monitored by means of UDP over either a wired or wireless internet/intranet connection.
The TeensyNet uses DHCP to obtain an IP address, and a series of PHP-based web pages are used to monitor and control the TeensyNet.

This system was designed to be used on an local intranet, and is NOT secure if exposed to the outside world. Connect it to the internet at your own risk.

This repository contains both the TeensyNet.ino code and the web interface files.
