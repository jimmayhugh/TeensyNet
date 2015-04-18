/*  Teensy1Wire.h
    Version 0.02 11/22/2014
    by Jim Mayhugh
    
    11/22/2014 - Added 4x20 LCD 1-wire slave
*/
#ifndef T1W_H
#define T1W_H


char itoaBuf[20];
uint16_t rBuffCnt = 0;
char c;
uint8_t cnt = 0;
uint8_t chipSelected;
uint8_t actionSelected;
uint8_t setChipState;
uint8_t *chipAddrPtr;
bool serialMessageReady = FALSE;
bool actionPtrMatch = FALSE;
bool showCelsius = FALSE;
uint16_t packetSize;

uint32_t timer, timer2, startTime, endTime;
elapsedMillis udpTimer;
const uint32_t updateTime = 250;
const uint32_t ramUpdateTime = 10000;
const uint32_t ds2762UpdateTime = 250;
const uint32_t udpTimerMax = (1000 * 60 * 60); // no UDP traffic for 1 hour results in a reset
const uint8_t  dsDevice = 0x45;

// OneWire Setup;
// Family codes
const uint8_t t3tcID         = 0xAA; // Teensy 3.0 1-wire slave with MAX31855 K-type Thermocouple chip
const uint8_t dsLCD          = 0x47; // Teensy 3.x 1-wire slave 4x20 HD44780 LCD
const uint8_t dsGLCDP        = 0x45; // Teensy 3.1 1-wire slave 800x400 7" GLCD with Paging
const uint8_t dsGLCD         = 0x44; // Teensy 3.1 1-wire slave 800x400 7" GLCD
const uint8_t max31850ID     = 0x3B; // MAX31850 K-type Thermocouple chip
const uint8_t ds2762ID       = 0x30; // Maxim 2762 digital k-type thermocouple
const uint8_t ds18b20ID      = 0x28; // Maxim DS18B20 digital Thermometer device
const uint8_t ds2406ID       = 0x12; // Maxim DS2406+ digital switch

// DS2406+ Digital Switch Family Code and registers
const uint8_t dsPIO_A        = 0x20;
const uint8_t dsPIO_B        = 0x40;
const uint8_t ds2406MemWr    = 0x55;
const uint8_t ds2406MemRd    = 0xaa;
const uint8_t ds2406AddLow   = 0x07;
const uint8_t ds2406AddHi    = 0x00;
const uint8_t ds2406PIOAoff  = 0x3f;
const uint8_t ds2406PIOAon   = 0x1f;
const uint8_t ds2406End      = 0xff;


const uint8_t oneWireAddress = 2; // OneWire Bus Address - use pin 2 for TeensyNet board
const uint8_t chipAddrSize   = 8; // 64bit OneWire Address
const uint8_t chipNameSize   = 15;

#if __MK20DX128__
const uint8_t maxChips       = 12; // Maximum number of Chips
#elif __MK20DX256__
const uint8_t maxChips       = 36; // Maximum number of Chips
#endif

const uint32_t tempReadDelay = 125;

uint8_t chipAddrArray[chipAddrSize] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

const char *charChipAddrArray = "0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00";
const char *unassignedStr = "__UNASSIGNED___";
char *nullStr = (char *)"";


typedef struct
{
  uint8_t    chipAddr[chipAddrSize];
  int16_t    chipStatus;
  uint32_t   tempTimer;
  char       chipName[chipNameSize+1];
}chipStruct;

const chipStruct chipClear = { {0,0,0,0,0,0,0,0}, 0, 0, "" };

chipStruct chip[maxChips] = 
{
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
#if __MK20DX256__
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
#endif  
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" },
  { {0,0,0,0,0,0,0,0}, 0, 0, "" }
};


#endif
