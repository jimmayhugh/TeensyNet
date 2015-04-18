/*  TeensyVersionInfo.h
    Version 0.04 04/05/2015
    by Jim Mayhugh
*/
#ifndef TVI_H
#define TVI_H

#if __MK20DX128__
const char* teensyType = "Teensy3.0 ";
const char* versionStrName   = "TeensyNet 3.0";
#elif __MK20DX256__
const char* teensyType = "Teensy3.1 ";
const char* versionStrName   = "TeensyNet 3.1";
#else
const char* teensyType = "UNKNOWN ";
#endif

const char* versionStrNumber = "V-0.0.55";
const char* versionStrDate   = "04/10/2015";

const uint8_t boardVersion = 18; // board versions below 18 use reverse logic for LEDs and test points
const uint8_t ledON  = 0x00;
const uint8_t ledOFF = 0xFF;

/* Version History

V-0.0.45 - Added Serial2 debug
V-0.0.46 - Added TeensyNetLCD 1-wire 4x20 LCD Slave
V-0.0.49 - Split into multiple files
           Added 1-wire updates
V-0.0.51 - Changed temperature conversion in updateChipStatus() to integer arithmetic
           Added UDP call to change Temperature conversion (C or F)
V-0.0.53 - Added ability to change debug port on the fly
V-0.0.54 - Now uses included i2c_t3 library instead of FastWire
           Modified I2CEEPROMAnything.h to use i2c_t3 library.
V-0.0.55 - 
*/
#endif
