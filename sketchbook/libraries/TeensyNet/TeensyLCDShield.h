/*  TeensyLCDShield.h
    Version 0.01 0831/2014
    by Jim Mayhugh
*/
#ifndef TLCD_H
#define TLCD_H

// The shield uses the I2C SCL and SDA pins. 
// You can connect other I2C sensors to the I2C bus and share
// the I2C bus.

Teensy_RGBLCDShield LCD0 = Teensy_RGBLCDShield(0);
Teensy_RGBLCDShield LCD1 = Teensy_RGBLCDShield(1);
Teensy_RGBLCDShield LCD2 = Teensy_RGBLCDShield(2);
Teensy_RGBLCDShield LCD3 = Teensy_RGBLCDShield(3);
Teensy_RGBLCDShield LCD4 = Teensy_RGBLCDShield(4);
Teensy_RGBLCDShield LCD5 = Teensy_RGBLCDShield(5);
Teensy_RGBLCDShield LCD6 = Teensy_RGBLCDShield(6);
Teensy_RGBLCDShield LCD7 = Teensy_RGBLCDShield(7);

Teensy_RGBLCDShield *lcd[] = { &LCD0, &LCD1, &LCD2, &LCD3, &LCD4, &LCD5, &LCD6, &LCD7 };

// These #defines make it easy to set the backlight color
#define BL_RED 0x1
#define BL_YELLOW 0x3
#define BL_GREEN 0x2
#define BL_TEAL 0x6
#define BL_BLUE 0x4
#define BL_VIOLET 0x5
#define BL_WHITE 0x7

uint8_t const lcdChars = 20;
uint8_t const lcdRows  = 4;
uint8_t const numLCDs  = 8;

char lcdStr[lcdChars + 1];
char lcdStrBuf[lcdChars + 1];

// End LCD Stuff
#endif
