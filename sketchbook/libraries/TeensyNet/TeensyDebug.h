/*  TeensyDebug.h
    Version 0.0.2 04/04/2015
    by Jim Mayhugh
*/
#ifndef TDB_H
#define TDB_H

const uint32_t resetDebug      = 0x00000001;             // 0x00000001; //        1
const uint32_t pidDebug        = (resetDebug      << 1); // 0x00000002; //        2
const uint32_t eepromDebug     = (pidDebug        << 1); // 0x00000004; //        4
const uint32_t chipDebug       = (eepromDebug     << 1); // 0x00000008; //        8
const uint32_t findChipDebug   = (chipDebug       << 1); // 0x00000010; //       16
const uint32_t serialDebug     = (findChipDebug   << 1); // 0x00000020; //       32
const uint32_t udpDebug        = (serialDebug     << 1); // 0x00000040; //       64
const uint32_t wifiDebug       = (udpDebug        << 1); // 0x00000080; //      128
const uint32_t udpHexBuff      = (wifiDebug       << 1); // 0x00000100; //      256
const uint32_t chipNameDebug   = (udpHexBuff      << 1); // 0x00000200; //      512
const uint32_t actionDebug     = (chipNameDebug   << 1); // 0x00000400; //      1024
const uint32_t lcdDebug        = (actionDebug     << 1); // 0x00000800; //      2048
const uint32_t crcDebug        = (lcdDebug        << 1); // 0x00001000; //      4096
const uint32_t ds2762Debug     = (crcDebug        << 1); // 0x00002000; //      8192
const uint32_t bonjourDebug    = (ds2762Debug     << 1); // 0x00004000; //     16384
const uint32_t ethDebug        = (bonjourDebug    << 1); // 0x00008000; //     32768
const uint32_t udpTimerDebug   = (ethDebug        << 1); // 0x00010000; //     65536
const uint32_t glcdSerialDebug = (udpTimerDebug   << 1); // 0x00020000; //    131072 GLCD debug on serial port
const uint32_t glcdDebug       = (glcdSerialDebug << 1); // 0x00040000; //    262144 GLCD debug on GLCD
const uint32_t wdDebug         = (glcdDebug       << 1); // 0x00080000; //    524288 Watchdog 
const uint32_t chipStatusLED   = (wdDebug         << 1); // 0x00100000; //   1048576 ChipStatus LED
const uint32_t glcd1WLED       = (chipStatusLED   << 1); // 0x00200000; //   2097152 glcd1W LED
const uint32_t udpProcessLED   = (glcd1WLED       << 1); // 0x00400000; //   4194304 udpProcess LED
const uint32_t glcdNameUpdate  = (udpProcessLED   << 1); // 0x00800000; //   8388608 update GLCD Name
const uint32_t lcdSerialDebug  = (glcdNameUpdate  << 1); // 0x01000000; //  16777216 1-wire LCD Debug on
const uint32_t lcdNameUpdate   = (lcdSerialDebug  << 1); // 0x02000000; //  33554432 1-wire LCD Name Debug on
const uint32_t debugLCD        = (lcdNameUpdate   << 1); // 0x04000000; //  67108864 LCD debug on
const uint32_t lcd1wLED        = (debugLCD        << 1); // 0x08000000; // 134217728 lcd1W LED

Stream *myDebug[] = { &Serial, &Serial2 };
uint8_t debugPort = 0;

//uint32_t setDebug = 0x00000000;
uint32_t setDebug = (chipStatusLED | udpProcessLED | glcd1WLED | lcd1wLED | chipDebug); // default - runs status LEDs
//uint32_t setDebug = (eepromDebug | findChipDebug);
//uint32_t setDebug = (glcdSerialDebug | glcdDebug | lcdSerialDebug | lcdDebug | debugLCD);
//uint32_t setDebug = glcdSerialDebug;
//uint32_t setDebug = glcdDebug;
//uint32_t setDebug = glcd1WLED;
//uint32_t setDebug = udpProcessLED;
//uint32_t setDebug = (udpProcessLED | glcdDebug);
//uint32_t setDebug = (chipStatusLED | glcd1WLED | glcdNameUpdate | udpProcessLED | findChipDebug);
//uint32_t setDebug = (chipStatusLED | glcd1WLED | udpProcessLED | lcdNameUpdate | debugLCD |eepromDebug);
//uint32_t setDebug = (lcdNameUpdate | debugLCD | glcdNameUpdate | glcdDebug);
//uint32_t setDebug = (glcdNameUpdate | glcdSerialDebug | lcdNameUpdate | lcdSerialDebug);
#endif

