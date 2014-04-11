/********************

TeensyNet.ino

Version 0.0.32
Last Modified 04/10/2014
By Jim Mayhugh

Uses the 24LC512 EEPROM for structure storage, and Teensy 3.1 board

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

This software uses multiple libraries that are subject to additional
licenses as defined by the author of that software. It is the user's
and developer's responsibility to determine and adhere to any additional
requirements that may arise.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


*********************/

/*********************
This code requires several changes in several libraries to work properly. This is due primarily to the limite amount of
RAM that's available in the Arduino series versus the Teensy.

------------------------------------------
/arduino/libraries/Ethernet/EthernetUDP.h:
replace:

#define UDP_TX_PACKET_MAX_SIZE 24

with:

#define UDP_TX_PACKET_MAX_SIZE 2048

------------------------------------------

------------------------------------------
/arduino/libraries/Wire/Wire.cpp:
in the function TwoWire::begin(void);
replace:

#if F_BUS == 48000000
	I2C0_F = 0x27;	// 100 kHz
	// I2C0_F = 0x1A; // 400 kHz
	// I2C0_F = 0x0D; // 1 MHz
	I2C0_FLT = 4;
#elif F_BUS == 24000000
	I2C0_F = 0x1F; // 100 kHz
	// I2C0_F = 0x45; // 400 kHz
	// I2C0_F = 0x02; // 1 MHz
	I2C0_FLT = 2;

with:

#if F_BUS == 48000000
	//I2C0_F = 0x27;	// 100 kHz
	I2C0_F = 0x1A; // 400 kHz
	// I2C0_F = 0x0D; // 1 MHz
	I2C0_FLT = 4;
#elif F_BUS == 24000000
	// I2C0_F = 0x1F; // 100 kHz
	I2C0_F = 0x45; // 400 kHz
	// I2C0_F = 0x02; // 1 MHz
	I2C0_FLT = 2;


/arduino/libraries/Wire/Wire.h
replace

#define BUFFER_LENGTH 32

with

#define BUFFER_LENGTH 130


*********************/

#include <PID_v1.h>
#include <math.h>
#include <EEPROM.h>
//#include "EEPROMAnything.h"
#include <OneWire.h>
#include <errno.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>  // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <EthernetBonjour.h>
#include <stdio.h>
#include <stdlib.h>
#include <t3mac.h>
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>
#include "I2CEEPROMAnything.h"
#include "TeensyNet.h" // function prototype list

/*
  General Setup
*/

#if __MK20DX128__
const char* teensyType = "Teensy3.0 ";
const char* versionStrName   = "TeensyNet 3.0";
#elif __MK20DX256__
const char* teensyType = "Teensy3.1 ";
const char* versionStrName   = "TeensyNet 3.1";
#else
const char* teensyType = "UNKNOWN ";
#endif

const char* versionStrNumber = "V-0.0.32";
const char* versionStrDate   = "04/10/2014";

// Should restart Teensy 3, will also disconnect USB during restart

// From http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337e/Cihcbadd.html
// Search for "0xE000ED0C"
// Original question http://forum.pjrc.com/threads/24304-_reboot_Teensyduino%28%29-vs-_restart_Teensyduino%28%29?p=35981#post35981

#define RESTART_ADDR       0xE000ED0C
#define READ_RESTART()     (*(volatile uint32_t *)RESTART_ADDR)
#define WRITE_RESTART(val) ((*(volatile uint32_t *)RESTART_ADDR) = (val))


const uint32_t resetDebug      = 0x00000001; //      1
const uint32_t pidDebug        = 0x00000002; //      2
const uint32_t eepromDebug     = 0x00000004; //      4
const uint32_t chipDebug       = 0x00000008; //      8
const uint32_t findChipDebug   = 0x00000010; //     16
const uint32_t serialDebug     = 0x00000020; //     32
const uint32_t udpDebug        = 0x00000040; //     64
const uint32_t wifiDebug       = 0x00000080; //    128
const uint32_t udpHexBuff      = 0x00000100; //    256
const uint32_t chipNameDebug   = 0x00000200; //    512
const uint32_t actionDebug     = 0x00000400; //   1024
const uint32_t lcdDebug        = 0x00000800; //   2048
const uint32_t crcDebug        = 0x00001000; //   4096
const uint32_t ds2762Debug     = 0x00002000; //   8192
const uint32_t bonjourDebug    = 0x00004000; //  16384

uint32_t setDebug = 0x00000000;

const uint8_t resetWIZ5200pin  = 9;
const uint8_t chipStartPin     = 12;
const uint8_t resetWIZ5100pin  = 23;

// define serial commands

const uint8_t getMaxChips        = '1';
const uint8_t showChip           = getMaxChips + 1;    // "2"
const uint8_t getChipCount       = showChip + 1;       // "3"
const uint8_t getChipAddress     = getChipCount + 1;   // "4"
const uint8_t getChipStatus      = getChipAddress + 1; // "5"
const uint8_t setSwitchState     = getChipStatus + 1;  // "6"
const uint8_t getAllStatus       = setSwitchState + 1; // "7"
const uint8_t getChipType        = getAllStatus + 1;   // "8"
const uint8_t updateBonjour      = getChipType + 1;    // "9"

const uint8_t getActionArray     = 'A'; // start of new serial command list
const uint8_t updateActionArray  = getActionArray + 1;    // "B"
const uint8_t getActionStatus    = updateActionArray + 1; // "C"
const uint8_t getMaxActions      = getActionStatus + 1;   // "D"
const uint8_t setActionSwitch    = getMaxActions + 1;     // "E"
const uint8_t saveToEEPROM       = setActionSwitch + 1;   // "F"
const uint8_t getEEPROMstatus    = saveToEEPROM + 1;      // "G"
const uint8_t getNewSensors      = getEEPROMstatus + 1;   // "H"
const uint8_t masterStop         = getNewSensors + 1;     // "I"
const uint8_t getMaxPids         = masterStop + 1;        // "J"
const uint8_t masterPidStop      = getMaxPids + 1;        // "K"
const uint8_t getPidStatus       = masterPidStop + 1;     // "L"
const uint8_t updatePidArray     = getPidStatus + 1;      // "M"
const uint8_t getPidArray        = updatePidArray + 1;    // "N"
const uint8_t setPidArray        = getPidArray + 1;       // "O"
const uint8_t useDebug           = setPidArray + 1;       // "P"
const uint8_t restoreStructures  = useDebug + 1;          // "Q"
const uint8_t shortShowChip      = restoreStructures + 1; // "R"
const uint8_t updateChipName     = shortShowChip + 1;     // "S"
const uint8_t showActionStatus   = updateChipName + 1;    // "T"
const uint8_t setAction          = showActionStatus + 1;  // "U"

const uint8_t displayMessage     = 'w';
const uint8_t clearAndReset      = 'x';
const uint8_t clearEEPROM        = 'y';
const uint8_t versionID          = 'z';


// end of serial commands

const uint8_t softSerialError  = 'X';
const uint8_t setSwitchON      = 'N';
const uint8_t setSwitchOFF     = 'F';
const uint8_t switchStatusON   = 'N';
const uint8_t switchStatusOFF  = 'F';
const uint8_t tooHotSwitch     = 'H';
const uint8_t tooColdSwitch    = 'C';
const uint8_t noChipPresent    = 0xFF;


const long baudRate = 115200;

//char PartialBuffer[UDP_TX_PACKET_MAX_SIZE]; // Partial Buffer for oversized messages

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

// OneWire Setup;
const uint8_t oneWireAddress = 2; // OneWire Bus Address - use pin 2 for TeensyNet board
const uint8_t chipAddrSize   = 8; // 64bit OneWire Address
const uint8_t chipNameSize   = 16;
const uint8_t ds2406MemWr    = 0x55;
const uint8_t ds2406MemRd    = 0xaa;
const uint8_t ds2406AddLow   = 0x07;
const uint8_t ds2406AddHi    = 0x00;
const uint8_t ds2406PIOAoff  = 0x3f;
const uint8_t ds2406PIOAon   = 0x1f;
const uint8_t ds2406End      = 0xff;
const uint8_t t3tcID         = 0xAA; // Teensy 3.0 1-wire slave with MAX31855 K-type Thermocouple chip
const uint8_t max31850ID     = 0x3B; // MAX31850 K-type Thermocouple chip
const uint8_t ds2762ID       = 0x30; // Maxim 2762 digital k-type thermocouple
const uint8_t ds18b20ID      = 0x28; // Maxim DS18B20 digital Thermometer device
const uint8_t ds2406ID       = 0x12; // Maxim DS2406+ digital switch
const uint8_t dsPIO_A        = 0x20;
const uint8_t dsPIO_B        = 0x40;

const uint8_t maxChips       = 36; // Maximum number of Chips
const uint8_t maxActions     = 12; // Maximum number of Actions

OneWire  ds(oneWireAddress);

// DS2762 oneWire conversion table - K-Type Thermocouple

const uint16_t kNegTableCnt = 271;
PROGMEM prog_uint16_t kNegTable[kNegTableCnt] =
{
 /*       --0--  --1--  --2--  --3--  --4--  --5--  --6--  --7--  --8--  --9-- */
 /*   0*/     0,    39,    79,   118,   157,   197,   236,   275,   314,   353,
 /* -10*/   392,   431,   470,   508,   547,   586,   624,   663,   701,   739,
 /* -20*/   778,   816,   854,   892,   930,   968,  1006,  1043,  1081,  1119,
 /* -30*/  1156,  1194,  1231,  1268,  1305,  1343,  1380,  1417,  1453,  1490,
 /* -40*/  1527,  1564,  1600,  1637,  1673,  1709,  1745,  1782,  1818,  1854,
 /* -50*/  1889,  1925,  1961,  1996,  2032,  2067,  2103,  2138,  2173,  2208,
 /* -60*/  2243,  2278,  2312,  2347,  2382,  2416,  2450,  2485,  2519,  2553,
 /* -70*/  2587,  2620,  2654,  2688,  2721,  2755,  2788,  2821,  2854,  2887,
 /* -80*/  2920,  2953,  2986,  3018,  3050,  3083,  3115,  3147,  3179,  3211,
 /* -90*/  3243,  3274,  3306,  3337,  3368,  3400,  3431,  3462,  3492,  3523,
 /*-100*/  3554,  3584,  3614,  3645,  3675,  3705,  3734,  3764,  3794,  3823,
 /*-110*/  3852,  3882,  3911,  3939,  3968,  3997,  4025,  4054,  4082,  4110,
 /*-120*/  4138,  4166,  4194,  4221,  4249,  4276,  4303,  4330,  4357,  4384,
 /*-130*/  4411,  4437,  4463,  4490,  4516,  4542,  4567,  4593,  4618,  4644,
 /*-140*/  4669,  4694,  4719,  4744,  4768,  4793,  4817,  4841,  4865,  4889,
 /*-150*/  4913,  4936,  4960,  4983,  5006,  5029,  5052,  5074,  5097,  5119,
 /*-160*/  5141,  5163,  5185,  5207,  5228,  5250,  5271,  5292,  5313,  5333,
 /*-170*/  5354,  5374,  5395,  5415,  5435,  5454,  5474,  5493,  5512,  5531,
 /*-180*/  5550,  5569,  5588,  5606,  5624,  5642,  5660,  5678,  5695,  5713,
 /*-190*/  5730,  5747,  5763,  5780,  5797,  5813,  5829,  5845,  5861,  5876,
 /*-200*/  5891,  5907,  5922,  5936,  5951,  5965,  5980,  5994,  6007,  6021,
 /*-210*/  6035,  6048,  6061,  6074,  6087,  6099,  6111,  6123,  6135,  6147,
 /*-220*/  6158,  6170,  6181,  6192,  6202,  6213,  6223,  6233,  6243,  6252,
 /*-230*/  6262,  6271,  6280,  6289,  6297,  6306,  6314,  6322,  6329,  6337,
 /*-240*/  6344,  6351,  6358,  6364,  6370,  6377,  6382,  6388,  6393,  6399,
 /*-250*/  6404,  6408,  6413,  6417,  6421,  6425,  6429,  6432,  6435,  6438,
 /*-260*/  6411,  6444,  6446,  6448,  6450,  6452,  6453,  6455,  6456,  6457,
 /*-270*/  6458
};

const uint16_t kTableCnt = 1373;
PROGMEM prog_uint16_t kTable[kTableCnt] =
{
  /*       --0--  --1--  --2--  --3--  --4--  --5--  --6--  --7--  --8--  --9-- */
  /*0000*/     0,    39,    79,   119,   158,   198,   238,   277,   317,   357,
  /*0010*/   397,   437,   477,   517,   557,   597,   637,   677,   718,   758,
  /*0020*/   798,   838,   879,   919,   960,  1000,  1040,  1080,  1122,  1163,
  /*0030*/  1203,  1244,  1284,  1326,  1366,  1407,  1448,  1489,  1530,  1570,
  /*0040*/  1612,  1653,  1694,  1735,  1776,  1816,  1858,  1899,  1941,  1982,
  /*0050*/  2023,  2064,  2105,  2146,  2188,  2230,  2270,  2311,  2354,  2395,
  /*0060*/  2436,  2478,  2519,  2560,  2601,  2644,  2685,  2726,  2767,  2810,
  /*0070*/  2850,  2892,  2934,  2976,  3016,  3059,  3100,  3141,  3184,  3225,
  /*0080*/  3266,  3307,  3350,  3391,  3432,  3474,  3516,  3557,  3599,  3640,
  /*0090*/  3681,  3722,  3765,  3806,  3847,  3888,  3931,  3972,  4012,  4054,
  /*0100*/  4096,  4137,  4179,  4219,  4261,  4303,  4344,  4384,  4426,  4468,
  /*0110*/  4509,  4549,  4591,  4633,  4674,  4714,  4756,  4796,  4838,  4878,
  /*0120*/  4919,  4961,  5001,  5043,  5083,  5123,  5165,  5206,  5246,  5288,
  /*0130*/  5328,  5368,  5410,  5450,  5490,  5532,  5572,  5613,  5652,  5693,
  /*0140*/  5735,  5775,  5815,  5865,  5895,  5937,  5977,  6017,  6057,  6097,
  /*0150*/  6137,  6179,  6219,  6259,  6299,  6339,  6379,  6419,  6459,  6500,
  /*0160*/  6540,  6580,  6620,  6660,  6700,  6740,  6780,  6820,  6860,  6900,
  /*0170*/  6940,  6980,  7020,  7059,  7099,  7139,  7179,  7219,  7259,  7299,
  /*0180*/  7339,  7379,  7420,  7459,  7500,  7540,  7578,  7618,  7658,  7698,
  /*0190*/  7738,  7778,  7819,  7859,  7899,  7939,  7979,  8019,  8058,  8099,
  /*0200*/  8137,  8178,  8217,  8257,  8298,  8337,  8378,  8417,  8458,  8499,
  /*0210*/  8538,  8579,  8618,  8659,  8698,  8739,  8778,  8819,  8859,  8900,
  /*0220*/  8939,  8980,  9019,  9060,  9101,  9141,  9180,  9221,  9262,  9301,
  /*0230*/  9343,  9382,  9423,  9464,  9503,  9544,  9585,  9625,  9666,  9707,
  /*0240*/  9746,  9788,  9827,  9868,  9909,  9949,  9990, 10031, 10071, 10112,
  /*0250*/ 10153, 10194, 10234, 10275, 10316, 10356, 10397, 10439, 10480, 10519,
  /*0260*/ 10560, 10602, 10643, 10683, 10724, 10766, 10807, 10848, 10888, 10929,
  /*0270*/ 10971, 11012, 11053, 11093, 11134, 11176, 11217, 11259, 11300, 11340,
  /*0280*/ 11381, 11423, 11464, 11506, 11547, 11587, 11630, 11670, 11711, 11753,
  /*0290*/ 11794, 11836, 11877, 11919, 11960, 12001, 12043, 12084, 12126, 12167,
  /*0300*/ 12208, 12250, 12291, 12333, 12374, 12416, 12457, 12499, 12539, 12582,
  /*0310*/ 12624, 12664, 12707, 12747, 12789, 12830, 12872, 12914, 12955, 12997,
  /*0320*/ 13039, 13060, 13122, 13164, 13205, 13247, 13289, 13330, 13372, 13414,
  /*0330*/ 13457, 13497, 13539, 13582, 13624, 13664, 13707, 13749, 13791, 13833,
  /*0340*/ 13874, 13916, 13958, 14000, 14041, 14083, 14125, 14166, 14208, 14250,
  /*0350*/ 14292, 14335, 14377, 14419, 14461, 14503, 14545, 14586, 14628, 14670,
  /*0360*/ 14712, 14755, 14797, 14839, 14881, 14923, 14964, 15006, 15048, 15090,
  /*0370*/ 15132, 15175, 15217, 15259, 15301, 15343, 15384, 15426, 15468, 15510,
  /*0380*/ 15554, 15596, 15637, 15679, 15721, 15763, 15805, 15849, 15891, 15932,
  /*0390*/ 15974, 16016, 16059, 16102, 16143, 16185, 16228, 16269, 16312, 16355,
  /*0400*/ 16396, 16439, 16481, 16524, 16565, 16608, 16650, 16693, 16734, 16777,
  /*0410*/ 16820, 16861, 16903, 16946, 16989, 17030, 17074, 17115, 17158, 17201,
  /*0420*/ 17242, 17285, 17327, 17370, 17413, 17454, 17496, 17539, 17582, 17623,
  /*0430*/ 17667, 17708, 17751, 17794, 17836, 17879, 17920, 17963, 18006, 18048,
  /*0440*/ 18091, 18134, 18176, 18217, 18260, 18303, 18346, 18388, 18431, 18472,
  /*0450*/ 18515, 18557, 18600, 18643, 18686, 18728, 18771, 18812, 18856, 18897,
  /*0460*/ 18940, 18983, 19025, 19068, 19111, 19153, 19196, 19239, 19280, 19324,
  /*0470*/ 19365, 19408, 19451, 19493, 19536, 19579, 19621, 19664, 19707, 19750,
  /*0480*/ 19792, 19835, 19876, 19920, 19961, 20004, 20047, 20089, 20132, 20175,
  /*0490*/ 20218, 20260, 20303, 20346, 20388, 20431, 20474, 20515, 20559, 20602,
  /*0500*/ 20643, 20687, 20730, 20771, 20815, 20856, 20899, 20943, 20984, 21027,
  /*0510*/ 21071, 21112, 21155, 21199, 21240, 21283, 21326, 21368, 21411, 21454,
  /*0520*/ 21497, 21540, 21582, 21625, 21668, 21710, 21753, 21795, 21838, 21881,
  /*0530*/ 21923, 21966, 22009, 22051, 22094, 22137, 22178, 22222, 22265, 22306,
  /*0540*/ 22350, 22393, 22434, 22478, 22521, 22562, 22606, 22649, 22690, 22734,
  /*0550*/ 22775, 22818, 22861, 22903, 22946, 22989, 23032, 23074, 23117, 23160,
  /*0560*/ 23202, 23245, 23288, 23330, 23373, 23416, 23457, 23501, 23544, 23585,
  /*0570*/ 23629, 23670, 23713, 23757, 23798, 23841, 23884, 23926, 23969, 24012,
  /*0580*/ 24054, 24097, 24140, 24181, 24225, 24266, 24309, 24353, 24394, 24437,
  /*0590*/ 24480, 24523, 24565, 24608, 24650, 24693, 24735, 24777, 24820, 24863,
  /*0600*/ 24905, 24948, 24990, 25033, 25075, 25118, 25160, 25203, 25245, 25288,
  /*0610*/ 25329, 25373, 25414, 25457, 25500, 25542, 25585, 25626, 25670, 25711,
  /*0620*/ 25755, 25797, 25840, 25882, 25924, 25967, 26009, 26052, 26094, 26136,
  /*0630*/ 26178, 26221, 26263, 26306, 26347, 26390, 26432, 26475, 26516, 26559,
  /*0640*/ 26602, 26643, 26687, 26728, 26771, 26814, 26856, 26897, 26940, 26983,
  /*0650*/ 27024, 27067, 27109, 27152, 27193, 27236, 27277, 27320, 27362, 27405,
  /*0660*/ 27447, 27489, 27531, 27574, 27616, 27658, 27700, 27742, 27784, 27826,
  /*0670*/ 27868, 27911, 27952, 27995, 28036, 28079, 28120, 28163, 28204, 28246,
  /*0680*/ 28289, 28332, 28373, 28416, 28416, 28457, 28500, 28583, 28626, 28667,
  /*0690*/ 28710, 28752, 28794, 28835, 28877, 28919, 28961, 29003, 29045, 29087,
  /*0700*/ 29129, 29170, 29213, 29254, 29297, 29338, 29379, 29422, 29463, 29506,
  /*0710*/ 29548, 29589, 29631, 29673, 29715, 29757, 29798, 29840, 29882, 29923,
  /*0720*/ 29964, 30007, 30048, 30089, 30132, 30173, 30214, 30257, 30298, 30341,
  /*0730*/ 30382, 30423, 30466, 30507, 30548, 30589, 30632, 30673, 30714, 30757,
  /*0740*/ 30797, 30839, 30881, 30922, 30963, 31006, 31047, 31088, 31129, 31172,
  /*0750*/ 31213, 31254, 31295, 31338, 31379, 31420, 31461, 31504, 31545, 31585,
  /*0760*/ 31628, 31669, 31710, 31751, 31792, 31833, 31876, 31917, 31957, 32000,
  /*0770*/ 32040, 32082, 32124, 32164, 32206, 32246, 32289, 32329, 32371, 32411,
  /*0780*/ 32453, 32495, 32536, 32577, 32618, 32659, 32700, 32742, 32783, 32824,
  /*0790*/ 32865, 32905, 32947, 32987, 33029, 33070, 33110, 33152, 33192, 33234,
  /*0800*/ 33274, 33316, 33356, 33398, 33439, 33479, 33521, 33561, 33603, 33643,
  /*0810*/ 33685, 33725, 33767, 33807, 33847, 33889, 33929, 33970, 34012, 34052,
  /*0820*/ 34093, 34134, 34174, 34216, 34256, 34296, 34338, 34378, 34420, 34460,
  /*0830*/ 34500, 34542, 34582, 34622, 34664, 34704, 34744, 34786, 34826, 34866,
  /*0840*/ 34908, 34948, 34999, 35029, 35070, 35109, 35151, 35192, 35231, 35273,
  /*0850*/ 35313, 35353, 35393, 35435, 35475, 35515, 35555, 35595, 35637, 35676,
  /*0860*/ 35718, 35758, 35798, 35839, 35879, 35920, 35960, 36000, 36041, 36081,
  /*0870*/ 36121, 36162, 36202, 36242, 36282, 36323, 36363, 36403, 36443, 36484,
  /*0880*/ 36524, 36564, 36603, 36643, 36685, 36725, 36765, 36804, 36844, 36886,
  /*0890*/ 36924, 36965, 37006, 37045, 37085, 37125, 37165, 37206, 37246, 37286,
  /*0900*/ 37326, 37366, 37406, 37446, 37486, 37526, 37566, 37606, 37646, 37686,
  /*0910*/ 37725, 37765, 37805, 37845, 37885, 37925, 37965, 38005, 38044, 38084,
  /*0920*/ 38124, 38164, 38204, 38243, 38283, 38323, 38363, 38402, 38442, 38482,
  /*0930*/ 38521, 38561, 38600, 38640, 38679, 38719, 38759, 38798, 38838, 38878,
  /*0940*/ 38917, 38957, 38996, 39036, 39076, 39115, 39164, 39195, 39234, 39274,
  /*0950*/ 39314, 39353, 39393, 39432, 39470, 39511, 39549, 39590, 39628, 39668,
  /*0960*/ 39707, 39746, 39786, 39826, 39865, 39905, 39944, 39984, 40023, 40061,
  /*0970*/ 40100, 40140, 40179, 40219, 40259, 40298, 40337, 40375, 40414, 40454,
  /*0980*/ 40493, 40533, 40572, 40610, 40651, 40689, 40728, 40765, 40807, 40846,
  /*0990*/ 40885, 40924, 40963, 41002, 41042, 41081, 41119, 41158, 41198, 41237,
  /*1000*/ 41276, 41315, 41354, 41393, 41431, 41470, 41509, 41548, 41587, 41626,
  /*1010*/ 41665, 41704, 41743, 41781, 41820, 41859, 41898, 41937, 41976, 42014,
  /*1020*/ 42053, 42092, 42131, 42169, 42208, 42247, 42286, 42324, 42363, 42402,
  /*1030*/ 42440, 42479, 42518, 42556, 42595, 42633, 42672, 42711, 42749, 42788,
  /*1040*/ 42826, 42865, 42903, 42942, 42980, 43019, 43057, 43096, 43134, 43173,
  /*1050*/ 43211, 43250, 43288, 43327, 43365, 43403, 43442, 43480, 43518, 43557,
  /*1060*/ 43595, 43633, 43672, 43710, 43748, 43787, 43825, 43863, 43901, 43940,
  /*1070*/ 43978, 44016, 44054, 44092, 44130, 44169, 44207, 44245, 44283, 44321,
  /*1080*/ 44359, 44397, 44435, 44473, 44512, 44550, 44588, 44626, 44664, 44702,
  /*1090*/ 44740, 44778, 44816, 44853, 44891, 44929, 44967, 45005, 45043, 45081,
  /*1100*/ 45119, 45157, 45194, 45232, 45270, 45308, 45346, 45383, 45421, 45459,
  /*1110*/ 45497, 45534, 45572, 45610, 45647, 45685, 45723, 45760, 45798, 45836,
  /*1120*/ 45873, 45911, 45948, 45986, 46024, 46061, 46099, 46136, 46174, 46211,
  /*1130*/ 46249, 46286, 46324, 46361, 46398, 46436, 46473, 46511, 46548, 46585,
  /*1140*/ 46623, 46660, 46697, 46735, 46772, 46809, 46847, 46884, 46921, 46958,
  /*1150*/ 46995, 47033, 47070, 47107, 47144, 47181, 47218, 47256, 47293, 47330,
  /*1160*/ 47367, 47404, 47441, 47478, 47515, 47552, 47589, 47626, 47663, 47700,
  /*1170*/ 47737, 47774, 47811, 47848, 47884, 47921, 47958, 47995, 48032, 48069,
  /*1180*/ 48105, 48142, 48179, 48216, 48252, 48289, 48326, 48363, 48399, 48436,
  /*1190*/ 48473, 48509, 48546, 48582, 48619, 48656, 48692, 48729, 48765, 48802,
  /*1200*/ 48838, 48875, 48911, 48948, 48984, 49021, 49057, 49093, 49130, 49166,
  /*1210*/ 49202, 49239, 49275, 49311, 49348, 49384, 49420, 49456, 49493, 49529,
  /*1220*/ 49565, 49601, 49637, 49674, 49710, 49746, 49782, 49818, 49854, 49890,
  /*1230*/ 49926, 49962, 49998, 50034, 50070, 50106, 50142, 50178, 50214, 50250,
  /*1240*/ 50286, 50322, 50358, 50393, 50429, 50465, 50501, 50537, 50572, 50608,
  /*1250*/ 50644, 50680, 50715, 50751, 50787, 50822, 50858, 50894, 50929, 50965,
  /*1260*/ 51000, 51036, 51071, 51107, 51142, 51178, 51213, 51249, 51284, 51320,
  /*1270*/ 51355, 51391, 51426, 51461, 51497, 51532, 51567, 51603, 51638, 51673,
  /*1280*/ 51708, 51744, 51779, 51814, 51849, 51885, 51920, 51955, 51990, 52025,
  /*1290*/ 52060, 52095, 52130, 52165, 52200, 52235, 52270, 52305, 52340, 52375,
  /*1300*/ 52410, 52445, 52480, 52515, 52550, 52585, 52620, 52654, 52689, 52724,
  /*1310*/ 52759, 52794, 52828, 52863, 52898, 52932, 52967, 53002, 53037, 53071,
  /*1320*/ 53106, 53140, 53175, 53210, 53244, 53279, 53313, 53348, 53382, 53417,
  /*1330*/ 53451, 53486, 53520, 53555, 53589, 53623, 53658, 53692, 53727, 53761,
  /*1340*/ 53795, 53830, 53864, 53898, 53932, 53967, 54001, 54035, 54069, 54104,
  /*1350*/ 54138, 54172, 54206, 54240, 54274, 54308, 54343, 54377, 54411, 54445,
  /*1360*/ 54479, 54513, 54547, 54581, 54615, 54649, 54683, 54717, 54751, 54785,
  /*1370*/ 54819, 54852, 54886
};
uint8_t addr[8], voltage[2], cjTemp[2], error, sign, i;
int16_t tcVoltage, cjTemperature, tblLo, eePntr, tempC, cjComp;
uint16_t tcBuff, tblHi, testVal;

const uint32_t tempReadDelay = 125;

uint8_t chipAddrArray[chipAddrSize] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

const char *charChipAddrArray = "0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00";
const char *unassignedStr = "___UNASSIGNED___";


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

typedef struct
{
  bool       actionEnabled;
  chipStruct *tempPtr;
  int16_t    tooCold;
  chipStruct *tcPtr;
  uint8_t    tcSwitchLastState;
  uint32_t   tcDelay;
  uint32_t   tcMillis;
  int16_t    tooHot;
  chipStruct *thPtr;
  uint8_t    thSwitchLastState;
  uint32_t   thDelay;
  uint32_t   thMillis;
  uint8_t    lcdAddr;
  uint32_t   lcdMillis;
}chipActionStruct;

const chipActionStruct actionClear = { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 };

const uint32_t lcdUpdateTimer = 1000;

chipActionStruct action[maxActions] =
{
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 }
};

uint8_t chipBuffer[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t chipCnt, chipX = 0, actionsCnt = 0;


// PID Stuff

const uint8_t maxPIDs = 4;
uint8_t pidCnt = 0;

typedef struct
{
  bool       pidEnabled;
  chipStruct *tempPtr;
  double     pidSetPoint;
  chipStruct *switchPtr;
  double     pidKp;
  double     pidKi;
  double     pidKd;
  int        pidDirection;
  uint32_t   pidWindowSize;
  uint32_t   pidwindowStartTime;
  double     pidInput;
  double     pidOutput;
  PID       *myPID;
}chipPIDStruct;

const chipPIDStruct pidClear = { FALSE, NULL, 70, NULL, 0, 0, 0, 0, 5000, 0, 0, 0, NULL };

chipPIDStruct ePID[maxPIDs] =
{
  { FALSE, NULL, 70, NULL, 0, 0, 0, 0, 5000, 0, 0, 0, NULL },
  { FALSE, NULL, 70, NULL, 0, 0, 0, 0, 5000, 0, 0, 0, NULL },
  { FALSE, NULL, 70, NULL, 0, 0, 0, 0, 5000, 0, 0, 0, NULL },
  { FALSE, NULL, 70, NULL, 0, 0, 0, 0, 5000, 0, 0, 0, NULL }
};

//Specify the links and initial tuning parameters
PID PID0(&ePID[0].pidInput,   &ePID[0].pidOutput,  &ePID[0].pidSetPoint,  (double) ePID[0].pidKp,  (double) ePID[0].pidKi,  (double) ePID[0].pidKd,  ePID[0].pidDirection);
PID PID1(&ePID[1].pidInput,   &ePID[1].pidOutput,  &ePID[1].pidSetPoint,  (double) ePID[1].pidKp,  (double) ePID[1].pidKi,  (double) ePID[1].pidKd,  ePID[1].pidDirection);
PID PID2(&ePID[2].pidInput,   &ePID[2].pidOutput,  &ePID[2].pidSetPoint,  (double) ePID[2].pidKp,  (double) ePID[2].pidKi,  (double) ePID[2].pidKd,  ePID[2].pidDirection);
PID PID3(&ePID[3].pidInput,   &ePID[3].pidOutput,  &ePID[3].pidSetPoint,  (double) ePID[3].pidKp,  (double) ePID[3].pidKi,  (double) ePID[3].pidKd,  ePID[3].pidDirection);

PID *pidArrayPtr[] = {&PID0, &PID1, &PID2, &PID3};

// End PID Stuff


//I2CEEPROM Stuff
const uint32_t   I2CEEPROMsize         = 0xFFFF;   // MicroChip 24LC512
const uint16_t   I2CEEPROMidAddr       = 0x05;    // ID address to verify a previous I2CEEPROM write
const uint16_t   I2CEEPROMccAddr       = 0x10;    // number of chips found during findchips()
const uint16_t   I2CEEPROMbjAddr       = 0x50;    // start of Bonjour name buffer
const uint16_t   I2CEEPROMchipAddr     = 0x1000;  // start address of chip structures
const uint16_t   I2CEEPROMactionAddr   = 0x5000;  // start address of action structures
const uint16_t   I2CEEPROMpidAddr      = 0x9000;  // start address of chip structures
const uint8_t    I2CEEPROMidVal        = 0x55;    // Shows that an EEPROM update has occurred 
const uint8_t    I2C0x50               = 0x50;    // device address at 0x50
const uint8_t    I2C0x51               = 0x51;    // device address at 0x51
const uint8_t    pageSize              = 128;     // MicroChip 24LC512 buffer page
bool             i2cEepromReady        = FALSE;
uint16_t         i2cEeResult16;
uint8_t          i2cEeResult;

// Ethernet Stuff

// #define STATIC_IP // uncomment to use a static IP Address

// The IP address will be dependent on your local network:
// buffers for receiving and sending data

char PacketBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
char ReplyBuffer[UDP_TX_PACKET_MAX_SIZE];  // a string to send back


const uint8_t wizReset = 23;         // WIZ nic reset

unsigned int localPort = 2652;      // local port to listen on

// set up the static IP address if you want to use one
#ifdef STATIC_IP
IPAddress ip(192, 168, 1, 51);
#endif

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

elapsedMillis runBonjour;
const uint16_t runBonjourTimeout = 5000;
// char IPaddrBuf[17];
char bonjourBuf[35];
char bonjourNameBuf[chipNameSize];
// LCD Stuff

// The shield uses the I2C SCL and SDA pins. 
// You can connect other I2C sensors to the I2C bus and share
// the I2C bus.

Adafruit_RGBLCDShield LCD0 = Adafruit_RGBLCDShield(0);
Adafruit_RGBLCDShield LCD1 = Adafruit_RGBLCDShield(1);
Adafruit_RGBLCDShield LCD2 = Adafruit_RGBLCDShield(2);
Adafruit_RGBLCDShield LCD3 = Adafruit_RGBLCDShield(3);
Adafruit_RGBLCDShield LCD4 = Adafruit_RGBLCDShield(4);
Adafruit_RGBLCDShield LCD5 = Adafruit_RGBLCDShield(5);
Adafruit_RGBLCDShield LCD6 = Adafruit_RGBLCDShield(6);
Adafruit_RGBLCDShield LCD7 = Adafruit_RGBLCDShield(7);

Adafruit_RGBLCDShield *lcd[] = { &LCD0, &LCD1, &LCD2, &LCD3, &LCD4, &LCD5, &LCD6, &LCD7 };

// These #defines make it easy to set the backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

uint8_t const lcdChars = 20;
uint8_t const lcdRows  = 4;
uint8_t const numLCDs  = 8;

char lcdStr[lcdChars + 1];
char versionBuf[lcdChars + 1];

// End LCD Stuff

void setup()
{
  int x;
  Wire.begin();
  Serial.begin(baudRate);
  
  pinMode(chipStartPin, OUTPUT);
  digitalWrite(chipStartPin, HIGH); // sync pin for DSO
    
  delay(3000);
  
  Serial.print(F("Serial Debug starting at "));
  Serial.print(baudRate);
  Serial.println(F(" baud"));
  Serial.print(F("BUFFER_LENGTH = "));
  Serial.println(BUFFER_LENGTH);
  Serial.print(F("UDP_TX_PACKET_MAX_SIZE = "));
  Serial.println(UDP_TX_PACKET_MAX_SIZE);
  

  for(x = 0; x < numLCDs; x++)
  {
    lcd[x]->begin(lcdChars, lcdRows);
    lcd[x]->clear();
    lcd[x]->home();
    lcd[x]->print(F("Serial Debug = "));
    lcd[x]->print(baudRate);
    lcd[x]->setCursor(0, 1);
    lcd[x]->print(F("BUFFER_LENGTH = "));
    lcd[x]->print(BUFFER_LENGTH);
    lcd[x]->setCursor(0, 2);
    lcd[x]->print(F("UDP_PACKET_MAX_SIZE"));
    lcd[x]->setCursor(0, 3);
    lcd[x]->print(F("        "));
    lcd[x]->print(UDP_TX_PACKET_MAX_SIZE);
    lcd[x]->print(F("        "));
  }

  delay(3000);

  I2CEEPROM_readAnything(I2CEEPROMidAddr, i2cEeResult, I2C0x50);
  
  if(setDebug & eepromDebug)
  { 
    Serial.print(F("i2cEeResult = 0x"));
    Serial.println(i2cEeResult, HEX);
  }
  
  I2CEEPROM_readAnything(I2CEEPROMbjAddr, bonjourNameBuf, I2C0x50);
  
  if(bonjourNameBuf[0] >= 0x20 && bonjourNameBuf[0] <= 0x7a)
  { 
    if(setDebug & eepromDebug)
    { 
      Serial.print(F("bonjourNameBuf = "));
      Serial.println(bonjourNameBuf);
    }
  }else{
    for(int x = 0; x < chipNameSize; x++)
    {
      bonjourNameBuf[x] = 0x0;
    }
    if(setDebug & eepromDebug)
    { 
      Serial.println(F("bonjourNameBuf Cleared"));
    }
  }

  lcd[7]->clear();
  lcd[7]->home();
  
  if(i2cEeResult != 0x55)
  {
    if(setDebug & eepromDebug)
    { 
       Serial.println(F("No I2CEEPROM Data"));
    }

    lcd[7]->print(F(" No I2CEEPROM Data  "));
  
    i2cEepromReady = FALSE;
    findChips();
    saveStructures();
  }else{

    if(setDebug & eepromDebug)
    { 
      Serial.println(F("Getting EEPROM Data"));
    }

    lcd[7]->print(F("Valid I2CEEPROM Data"));
    
    I2CEEPROM_readAnything(I2CEEPROMccAddr, chipCnt, I2C0x50);
    
    if(setDebug & eepromDebug)
    { 
      Serial.print(F("chipCnt at I2CEEPROMccAddr =  "));
      Serial.println(chipCnt);
    }

    readStructures();
    if(setDebug & eepromDebug)
    { 
      Serial.println(F("I2CEEPROM Data Read Completed"));
    }

    lcd[7]->setCursor(0, 3);
    lcd[7]->print(F("I2CEEPROM Read Done "));
  
    i2cEepromReady = TRUE;
    
  }
  
  if(setDebug & eepromDebug)
  { 
    Serial.print( (sizeof(chipStruct) / sizeof(byte) ) * maxChips);
    Serial.println(F(" bytes in chip structure array"));
    Serial.print( (sizeof(chipActionStruct) / sizeof(byte) ) *maxActions);
    Serial.println(F(" bytes in action structure array"));
    Serial.print( (sizeof(chipPIDStruct) / sizeof(byte) ) *maxPIDs);
    Serial.println(F(" bytes in pid structure Array"));
  }

  delay(3000);
  
  if(setDebug & udpDebug)
  {
    Serial.println(F("Configuring IP"));
  }

  lcd[7]->clear();
  lcd[7]->home();
  lcd[7]->print(F("   Configuring IP   "));

  // start the Ethernet and UDP:
  read_mac();
  
  if(setDebug & udpDebug)
  {
    Serial.print(F("MAC Address = "));
    print_mac();
    Serial.println();
  }
  
  delay(2000);
  
#ifdef STATIC_IP  
  Ethernet.begin((uint8_t *) &mac, ip); // use this for static IP
  Udp.begin(localPort);
#else  
  if(Ethernet.begin((uint8_t *) &mac) == 0) // use this for dhcp
  {
    lcd[7]->setCursor(0, 1);
    lcd[7]->print(F("  IP Config Failed  "));
    lcd[7]->setCursor(0, 2);
    lcd[7]->print(F("Resetting TeensyNet "));
    Serial.println(F("Ethernet,begin() failed - Resetting TeensyNet"));
    delay(1000);
    softReset();
  }else{
    Serial.println(F("Ethernet,begin() success"));
    Udp.begin(localPort);

    Serial.print(F("My IP address: "));
    Serial.println(Ethernet.localIP());
    if(!(bonjourNameBuf[0] >= 0x20 && bonjourNameBuf[0] <= 0x7a))
    {
      sprintf(bonjourNameBuf, "TeensyNet%d", Ethernet.localIP()[3]);
    }
  }
#endif
// start Bonjour service
//  if(EthernetBonjour.begin("TeensyNetTURD"))
  if(EthernetBonjour.begin(bonjourNameBuf))
  {
    Serial.println(F("Bounjour Service started"));
    sprintf(bonjourBuf, "%s._discover", bonjourNameBuf);
    EthernetBonjour.addServiceRecord(bonjourBuf, localPort, MDNSServiceUDP);
    I2CEEPROM_writeAnything(I2CEEPROMbjAddr, bonjourNameBuf, I2C0x50);
  }else{
    Serial.println(F("Bounjour Service failed"));
  }
  EthernetBonjour.run();

// send startup data to status LCD (I2C address 0x27) if available  
  for(x = 0; x < numLCDs; x++)
  {
    lcd[x]->clear();
    lcd[x]->home();
    lcdCenterStr((char *) bonjourNameBuf);
    lcd[x]->print(lcdStr);
    lcd[x]->setCursor(0, 1);
    sprintf(versionBuf, "%s%s", teensyType, versionStrNumber);
    lcdCenterStr((char *) versionBuf);
    lcd[x]->print(lcdStr);
    lcd[x]->setCursor(0, 2);
    lcdCenterStr("My IP address is:");
    lcd[x]->print(lcdStr);
    lcd[x]->setCursor(0, 3);
    lcd[x]->print(F("   "));
    lcd[x]->print(Ethernet.localIP());
    lcd[x]->print(F("   "));
  }

  timer = millis();
  timer2 = millis();
  udpTimer = 0;
  
  action[0].lcdMillis = millis();
  action[1].lcdMillis = millis();
  action[2].lcdMillis = millis();
  action[3].lcdMillis = millis();
  
  for(int q = 0; q < UDP_TX_PACKET_MAX_SIZE; q++) // clear the buffer udp buffers
  {
    ReplyBuffer[q] = 0x00;
    PacketBuffer[q] = 0x00;
  }
  
  pidSetup();
}

void loop()
{
  packetSize = Udp.parsePacket();
  if(packetSize)
  {
    if(setDebug & udpDebug)
    {
      Serial.print(F("\nLoop:\nReceived packet of size: "));
      Serial.println(packetSize);
    }
    for(int pbSize = packetSize; pbSize < UDP_TX_PACKET_MAX_SIZE; pbSize++)
    {
      PacketBuffer[pbSize] = 0; // clear the remainder of the buffer
    }
    if(setDebug & udpDebug)
    {
      Serial.print("From ");
      IPAddress remote = Udp.remoteIP();
      for (int i =0; i < 4; i++)
      {
        Serial.print(remote[i], DEC);
        if (i < 3)
        {
          Serial.print(".");
        }
      }
      Serial.print(", port ");
      Serial.println(Udp.remotePort());
    }
    // read the packet into packetBufffer
    Udp.read(PacketBuffer,UDP_TX_PACKET_MAX_SIZE);
    if(setDebug & udpDebug)
    {
      Serial.println(F("Contents:"));
      Serial.println(PacketBuffer);
    }
    udpProcess();
  }

  if(runBonjour >= runBonjourTimeout)
  {
    if(setDebug & udpDebug)
    {
      Serial.println(F("EthernetBonjour.run()"));
    }
    runBonjour = runBonjour - runBonjourTimeout;
    EthernetBonjour.run();
  }
  
  if(timer > (millis() + 5000)) // in case of rollover
  {
    timer = millis();
  }
  
  for(int i = 0; i < maxChips; i++)
  {
    if(chip[i].tempTimer > (millis() + 5000)) // in case of rollover
    {
      chip[i].tempTimer = millis();
    }
  }
  
  updateChipStatus(chipX);
  chipX++;
  if(chipX >= maxChips){chipX = 0;}
  timer = millis();
  
  updateActions(actionsCnt);    
  actionsCnt++;
  if(actionsCnt >= maxActions){actionsCnt = 0;}

  updatePIDs(pidCnt);
  pidCnt++;
  if(pidCnt >= maxPIDs){pidCnt = 0;}

  if(udpTimer >= (1000 * 60 * 60 * 10))
  {
    MasterStop();
    softReset();
  }
}

void pidSetup(void)
{
/// ***** Start PID Setup *****

  for(int x =0; x < maxPIDs; x++)
  {
    ePID[x].myPID = pidArrayPtr[x];
  //tell the PID to range between 0 and the full window size
    ePID[x].myPID->SetOutputLimits(0, ePID[x].pidWindowSize);
    
    ePID[x].myPID->SetTunings(ePID[x].pidKp, ePID[x].pidKi, ePID[x].pidKd);
    
    ePID[x].myPID->SetSampleTime(200);

    if(ePID[x].pidDirection == 1)
    {
      ePID[x].myPID->SetControllerDirection(DIRECT);
    }else{
      ePID[x].myPID->SetControllerDirection(REVERSE);
    }
    
    
  //turn the PID on if variable are non-zero
    if( (ePID[x].pidEnabled == TRUE) &&
        (ePID[x].pidKp != 0) &&
        (ePID[x].pidKi != 0) &&
        (ePID[x].pidKd != 0) &&
        (ePID[x].pidWindowSize != 0)
        )
    {
      ePID[x].myPID->SetMode(AUTOMATIC);
    }else{
      ePID[x].pidEnabled = FALSE;
      ePID[x].myPID->SetMode(MANUAL);
    }
  }

}


void readStructures(void)
{  

  I2CEEPROM_readAnything(I2CEEPROMidAddr, i2cEeResult, I2C0x50);
  
  if(setDebug & eepromDebug)
  { 
    Serial.print(F("i2cEeResult = 0x"));
    Serial.println(i2cEeResult, HEX);
  }
  
  if(i2cEeResult != 0x55)
  {
    if(setDebug & eepromDebug)
    { 
       Serial.println(F("No EEPROM Data"));
    }
    i2cEepromReady = FALSE;
  }else{
    if(setDebug & eepromDebug)
    { 
       Serial.println(F("EEPROM Data Valid"));
    }
    i2cEepromReady = TRUE;
  }
  
  if(setDebug & eepromDebug)
  {
    Serial.println(F("Entering readStructures"));
    Serial.print(F("I2CEEPROMchipAddr = 0x"));
    Serial.println(I2CEEPROMchipAddr, HEX);
  }

  i2cEeResult16 = I2CEEPROM_readAnything(I2CEEPROMchipAddr, chip, I2C0x50);


  if(setDebug & eepromDebug)
  {
    Serial.print(F("Read "));
    Serial.print(i2cEeResult16);
    Serial.print(F(" bytes from address Ox"));
    Serial.println(I2CEEPROMchipAddr, HEX);
  }

  if(setDebug & eepromDebug)
  {
    Serial.print(F("I2CEEPROMactionAddr = 0x"));
    Serial.println(I2CEEPROMactionAddr, HEX);
  }

  i2cEeResult16 = I2CEEPROM_readAnything(I2CEEPROMactionAddr, action,  I2C0x50);

  if(setDebug & eepromDebug)
  {
    Serial.print(F("Read "));
    Serial.print(i2cEeResult16);
    Serial.print(F(" bytes from address Ox"));
    Serial.println(I2CEEPROMactionAddr, HEX);
  }

  if(setDebug & eepromDebug)
  {
    Serial.print(F("I2CEEPROMpidAddr = 0x"));
    Serial.println(I2CEEPROMpidAddr, HEX);
  }

  i2cEeResult16 = I2CEEPROM_readAnything(I2CEEPROMpidAddr, ePID,  I2C0x50);

  if(setDebug & eepromDebug)
  {
    Serial.print(F("Read "));
    Serial.print(i2cEeResult16);
    Serial.print(F(" bytes from address Ox"));
    Serial.println(I2CEEPROMpidAddr, HEX);
    Serial.println(F(" Completed"));
    Serial.println(F("Exiting readStructures"));
    Serial.println(F("Chip Structure"));
    displayStructure((byte *)(uint32_t) &chip, sizeof(chip), sizeof(chipStruct));
    Serial.println(F("Action Structure"));
    displayStructure((byte *)(uint32_t) &action, sizeof(action), sizeof(chipActionStruct));
    Serial.println(F("PID Structure"));
    displayStructure((byte *)(uint32_t) &ePID, sizeof(ePID), sizeof(chipPIDStruct));
  }

  pidSetup();
}


void saveStructures(void)
{  
  if(setDebug & eepromDebug)
  {
    Serial.println(F("Entering saveStructures"));
    Serial.print(F("I2CEEPROMchipAddr = 0x"));
    Serial.println(I2CEEPROMchipAddr, HEX);
  }
  I2CEEPROM_writeAnything(I2CEEPROMccAddr, chipCnt, I2C0x50);
  I2CEEPROM_writeAnything(I2CEEPROMidAddr, I2CEEPROMidVal, I2C0x50);
  i2cEeResult16 = I2CEEPROM_writeAnything(I2CEEPROMchipAddr, chip, I2C0x50);
  if(setDebug & eepromDebug)
  {
    Serial.print(F("Wrote "));
    Serial.print(i2cEeResult);
    Serial.print(F(" bytes to address Ox"));
    Serial.print(I2CEEPROMchipAddr, HEX);
    Serial.print(F(" from address 0x"));
    uint32_t chipStructAddr = (uint32_t) &chip[0];
    Serial.println(chipStructAddr, HEX);
  }

  if(setDebug & eepromDebug)
  {
    Serial.print(F("I2CEEPROMactionAddr = 0x"));
    Serial.println(I2CEEPROMactionAddr, HEX);
  }
  i2cEeResult16 = I2CEEPROM_writeAnything(I2CEEPROMactionAddr, action, I2C0x50);

  if(setDebug & eepromDebug)
  {
    Serial.print(F("Wrote "));
    Serial.print(i2cEeResult16);
    Serial.print(F(" bytes to address Ox"));
    Serial.println(I2CEEPROMactionAddr, HEX);
  }

  if(setDebug & eepromDebug)
  {
    Serial.print(F("I2CEEPROMpidAddr = 0x"));
    Serial.println(I2CEEPROMpidAddr, HEX);
  }
  i2cEeResult16 = I2CEEPROM_writeAnything(I2CEEPROMpidAddr, ePID, I2C0x50);

  if(setDebug & eepromDebug)
  {
    Serial.print(F("Wrote "));
    Serial.print(i2cEeResult16);
    Serial.print(F(" bytes to address Ox"));
    Serial.println(I2CEEPROMpidAddr, HEX);
    Serial.println(F(" Completed - Displaying chip Structures"));
    displayStructure((byte *)(uint32_t) &chip, sizeof(chip), sizeof(chipStruct));
    Serial.println(F("Action Structure"));
    displayStructure((byte *)(uint32_t) &action, sizeof(action), sizeof(chipActionStruct));
    Serial.println(F("PID Structure"));
    displayStructure((byte *)(uint32_t) &ePID, sizeof(ePID), sizeof(chipPIDStruct));
    Serial.println(F("Exiting saveStructures"));
  }
  I2CEEPROM_readAnything(I2CEEPROMidAddr, i2cEeResult, I2C0x50);
  
  if(setDebug & eepromDebug)
  { 
    Serial.print(F("i2cEeResult = 0x"));
    Serial.println(i2cEeResult, HEX);
  }
  
  if(i2cEeResult != 0x55)
  {
    if(setDebug & eepromDebug)
    { 
       Serial.println(F("EEPROM Data Erased"));
    }
    i2cEepromReady = FALSE;
  }else{
    if(setDebug & eepromDebug)
    { 
       Serial.println(F("EEPROM Data Valid"));
    }
    i2cEepromReady = TRUE;
  }
  
}

void displayStructure(byte *addr, int xSize, int ySize)
{
  int x, y;
  Serial.print(F("0x"));
  Serial.print((uint32_t)addr, HEX);
  Serial.print(F(": ")); 
  for(x = 0, y = 0; x < xSize; x++)
  {
    if(addr[x] >=0 && addr[x] <= 15)
    {
      Serial.print(F("0x0"));
    }else{
      Serial.print(F("0x"));
    }
    Serial.print(addr[x], HEX);
    y++;
    if(y < ySize)
    {
      Serial.print(F(", "));
    }else{
      y = 0;
      Serial.println();
      Serial.print(F("0x"));
      Serial.print((uint32_t)addr + x + 1, HEX);
      Serial.print(F(": ")); 
    }
  }
  Serial.println();
  Serial.println();
}


void updatePIDs(uint8_t pidCnt)
{
  
  if(ePID[pidCnt].pidEnabled == 1)
  {    
    // *** Start PID Loop ***
    ePID[pidCnt].pidInput = (double) ePID[pidCnt].tempPtr->chipStatus;

    if(setDebug & pidDebug)
    {
      Serial.println(F("Entering updatePIDs"));
      Serial.print(F("PID #"));
      Serial.println(pidCnt);
      Serial.print(F("ePID["));
      Serial.print(pidCnt);
      Serial.print(F("].pidInput = "));
      Serial.println((double) ePID[pidCnt].pidInput);
      Serial.print(F("ePID["));
      Serial.print(pidCnt);
      Serial.print(F("].pidKp = "));
      Serial.println(ePID[pidCnt].pidKp);
      Serial.print(F("ePID["));
      Serial.print(pidCnt);
      Serial.print(F("].pidKi = "));
      Serial.println(ePID[pidCnt].pidKi);
      Serial.print(F("ePID["));
      Serial.print(pidCnt);
      Serial.print(F("].pidKd = "));
      Serial.println(ePID[pidCnt].pidKd);
      Serial.print(F("ePID["));
      Serial.print(pidCnt);
      Serial.print(F("].pidDirection = "));
      Serial.println(ePID[pidCnt].pidDirection);
      Serial.print(F("ePID["));
      Serial.print(pidCnt);
      Serial.print(F("].pidWindowStartTime = "));
      Serial.println((uint32_t) ePID[pidCnt].pidwindowStartTime);
      Serial.print(F("millis() = "));
      Serial.println((uint32_t) millis());
    }
  
    if(ePID[pidCnt].myPID->Compute())
    {
      if(setDebug & pidDebug)
      {
        Serial.println(F("Compute() returned TRUE"));
      }
    }else{
      if(setDebug & pidDebug)
      {
        Serial.println(F("Compute() returned FALSE"));
      }
    }

    uint32_t now = millis();
    
    if(setDebug & pidDebug)
    {
      Serial.print(F("now - ePID[pidCnt].pidwindowStartTime = "));
      Serial.println(now - ePID[pidCnt].pidwindowStartTime);
    }

  /************************************************
   * turn the output pin on/off based on pid output
   ************************************************/
    if(now - ePID[pidCnt].pidwindowStartTime > ePID[pidCnt].pidWindowSize)
    { //time to shift the Relay Window
      ePID[pidCnt].pidwindowStartTime += ePID[pidCnt].pidWindowSize;
    }
  
      if(setDebug & pidDebug)
      {
        Serial.print(F("ePID["));
        Serial.print(pidCnt);
        Serial.print(F("].pidOutPut = "));
        Serial.println((double) ePID[pidCnt].pidOutput);
        Serial.print(F("now = "));
        Serial.println(now);
        Serial.print(F("ePID["));
        Serial.print(pidCnt);
        Serial.print(F("].pidwindowStartTime = "));
        Serial.println((double) ePID[pidCnt].pidwindowStartTime);
        Serial.print(F("now - ePID["));
        Serial.print(pidCnt);
        Serial.print(F("].pidwindowStartTime = "));
        Serial.println((double) now - ePID[pidCnt].pidwindowStartTime);
  
        Serial.print((double) ePID[pidCnt].pidOutput);
        
        if(ePID[pidCnt].pidOutput > now - ePID[pidCnt].pidwindowStartTime)
        {
          Serial.print(F(" > "));
        }else{
          Serial.print(F(" < "));
        }
        Serial.println((double) now - ePID[pidCnt].pidwindowStartTime);
      }

    if(ePID[pidCnt].pidOutput > now - ePID[pidCnt].pidwindowStartTime)
    {
      if(setDebug & pidDebug)
      {
        Serial.println(F("Turning Switch ON"));
      }
      actionSwitchSet((uint8_t *) &ePID[pidCnt].switchPtr->chipAddr, ds2406PIOAon);
    }else{
      if(setDebug & pidDebug)
      {
        Serial.println(F("Turning Switch OFF"));
      }
      actionSwitchSet((uint8_t *) &ePID[pidCnt].switchPtr->chipAddr, ds2406PIOAoff);
    }
  // *** End PID Loop ***

    if(setDebug & pidDebug)
      {
        Serial.print(F("ePID["));
        Serial.print(pidCnt);
        Serial.print(F("].pidOutput = "));
        Serial.println((double) ePID[pidCnt].pidOutput);
        Serial.println(F("Exiting updatePIDs"));
      }

  }else{
    ePID[pidCnt].myPID->SetMode(MANUAL);
  }
}

void findChips(void)
{
 int cntx = 0, cmpCnt, cmpArrayCnt, dupArray = 0, cnty;

  digitalWrite(chipStartPin, LOW); //start DSO sync

  ds.reset_search();
  delay(250);
  
  for(cnty = 0; cnty < maxChips; cnty++) // clear all chipnames
  {
    strcpy(chip[cnty].chipName, "");
  }
  
  while (ds.search(chip[cntx].chipAddr))
  {
    for(cmpCnt = 0; cmpCnt < cntx; cmpCnt++)
    {
      for(cmpArrayCnt = 0; cmpArrayCnt < chipAddrSize; cmpArrayCnt++)
      {
        if(chip[cntx].chipAddr[cmpArrayCnt] != chip[cmpCnt].chipAddr[cmpArrayCnt])
        {
          break;
        }else if(cmpArrayCnt == chipAddrSize-1){
          dupArray = 1;
        }
      }
    }
    
    if(dupArray == 1)
    {
      if(setDebug & findChipDebug)
      {
        Serial.print(F("Chip "));
        Serial.print(cntx);
        Serial.println(F(" - Duplicated Array"));
      }
      dupArray = 0;
      continue;
    }
    
    if(ds.crc8(chip[cntx].chipAddr, chipAddrSize-1) != chip[cntx].chipAddr[chipAddrSize-1])
    {
      continue;
      if(setDebug & findChipDebug)
      {
        Serial.print(F("CRC Error - Chip "));
        Serial.print(cntx);
        Serial.print(F(" = "));
        Serial.print(chip[cntx].chipAddr[chipAddrSize-1], HEX);
        Serial.print(F(", CRC should be "));
        Serial.println(ds.crc8(chip[cntx].chipAddr, chipAddrSize-1));
      }
    }

    if(setDebug & findChipDebug)
    {
      Serial.print(F("Chip "));
      Serial.print(cntx);
      Serial.print(F(" Address 0x"));
      Serial.print((uint32_t) &(chip[cntx].chipAddr), HEX);
      Serial.print(F(" = {"));
      
      for( int i = 0; i < chipAddrSize; i++)
      {
        if(chip[cntx].chipAddr[i]>=0 && chip[cntx].chipAddr[i]<16)
        {
          Serial.print(F("0x0"));
        }else{
          Serial.print(F("0x"));
        }
        Serial.print(chip[cntx].chipAddr[i], HEX);
        if(i < 7){Serial.print(F(","));}
      }
      Serial.println(F("}"));
    }
      
    cntx++;
    delay(50);
  }

  ds.reset_search();
  delay(250);
  chipCnt = cntx;
  if(cntx < maxChips)
  {
    for(;cntx<maxChips;cntx++)
    {
      for(int y=0;y<chipAddrSize;y++)
      {
        chip[cntx].chipAddr[y]=0;
      }
    }
  }
  
  if(setDebug & findChipDebug)
  {
    Serial.print(chipCnt);
    Serial.print(F(" Sensor"));
    if(chipCnt == 1)
    {
      Serial.println(F(" Detected"));
    }else{
      Serial.println(F("s Detected"));
    }
  }  
  digitalWrite(chipStartPin, HIGH); // end DSO sync
}

void sendUDPpacket(void)
{
  int v = 0, q = 0;
  
  if(setDebug & udpDebug)
  {
    Serial.print(F("rBuffCnt = "));
    Serial.println(rBuffCnt);
    Serial.println(F("ReplyBuffer:"));
    for(q = 0; q < rBuffCnt; q++)
    {
      if(ReplyBuffer[q] != 0x00)
      {
        Serial.write(ReplyBuffer[q]);
        if(ReplyBuffer[q] == ';')
        {
          Serial.println();
        }
      }
    }
    Serial.println();
    if(setDebug & udpHexBuff)
    {
      for(q = 0; q < rBuffCnt; q++)
      {
        Serial.print(F("0x"));
        if(ReplyBuffer[q] < 0x10)
        {
          Serial.print(F("0"));
        }
        Serial.print(ReplyBuffer[q], HEX);
        if(v <= 14)
        {
          Serial.print(F(" "));
          v++;
        }else{
          Serial.println();
          v = 0;
        }
      }
    } 
  }
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(ReplyBuffer);
  Udp.write("\n");
  Udp.endPacket();

  for(q = 0; q < UDP_TX_PACKET_MAX_SIZE; q++) // clear the udp buffers
  {
    ReplyBuffer[q] = 0x00;
    PacketBuffer[q] = 0x00;
  }  
}

void udpProcess(void)
{
  int x, ssBufOffset, pidArray, /*pidSection,*/ pidEnabledVal, pidDirectionVal;
  char *result = NULL, *addrResult = NULL/*, pidEnd = NULL*/;
  char delim[] = " ", addrDelim[] = ",";
  int16_t actionEnableTemp/*, pidEnableVal*/;
  int16_t resultCnt = 0, addrResultCnt = 0, actionArray = 0, actionSection= 0;
  uint32_t actionDelayVal;
  uint8_t addrVal[chipAddrSize],/* addrMatchCnt,*/ chipAddrCnt;

  rBuffCnt = 0;
  switch(PacketBuffer[0])
  {
    
    case getMaxChips: // "1"
    {
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d", maxChips);
      sendUDPpacket();
      break;
    }
    
    case showChip: // "2"
    {
      x = atoi((char *) &PacketBuffer[1]);
      if(x >= maxChips)
      {
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "ERROR");
      }else{
        showChipInfo(x);
      }
      sendUDPpacket();
      break;
    } 
    
    case getChipCount: // "3"
    {
      I2CEEPROM_readAnything(I2CEEPROMccAddr, chipCnt, I2C0x50);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d", chipCnt);
      sendUDPpacket();
      break;
    }
    
    case getChipAddress: // "4"
    {
      x = atoi((char *) &PacketBuffer[1]);
      if(x >= maxChips)
      {
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "ERROR");
      }else{
      showChipAddress((uint8_t *) &chip[x].chipAddr);
      }
      sendUDPpacket();
      break;
    }
    
    case getChipStatus: // "5"
    {
      x = atoi((char *) &PacketBuffer[1]);
     if(x >= maxChips)
      {
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "ERROR");
      }else{
        updateChipStatus(x);
        switch(chip[x].chipAddr[0])
        {
          case ds18b20ID:
          case t3tcID:
          case max31850ID:
          case ds2762ID:
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",(int) chip[x].chipStatus);
          }
          break;
  
          case ds2406ID:
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c",(char) chip[x].chipStatus);
          }
          break;
  
          default:
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","Z");
          }
          break;
        }
        rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s"," ");
        if(chip[x].chipName[0] == 0x00)
        {
          rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s",unassignedStr);
        }else{
          rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s", chip[x].chipName);
        }
      }
      sendUDPpacket();
      break;
    }

    case setSwitchState: // "6"
    {
     chipSelected = atoi((char *) &PacketBuffer[1]);
     if(chipSelected >= maxChips)
      {
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "ERROR");
      }else{
        if(chipSelected >= 10)
        {
          ssBufOffset = 3;
        }else{
          ssBufOffset = 2;
        }
        if(PacketBuffer[ssBufOffset] == setSwitchON)
        {
          setChipState = ds2406PIOAon;
        }else{
          setChipState = ds2406PIOAoff;
        }
        setSwitch(chipSelected, setChipState);
        updateChipStatus(chipSelected);
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c",(char) chip[chipSelected].chipStatus);
      }
      sendUDPpacket();
      break;
    }
    
    case getAllStatus: // "7"
    {
      for(int x = 0; x < maxChips; x++)
      {
        switch (chip[x].chipAddr[0])
        {
          case ds18b20ID:
          case ds2762ID:
          case max31850ID:
          case t3tcID:
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",(int) chip[x].chipStatus);
          }
          break;
          
          case ds2406ID:
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c",(char) chip[x].chipStatus);
          }
          break;
          
          default:
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","Z");
          }
          break;
        }
        if(x < maxChips -1)
        {
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",",");
        }
      }
      sendUDPpacket();
      break;
    }

    case getChipType: // "8"
    {
      x = atoi((char *) &PacketBuffer[1]);
      if(x >= maxChips)
      {
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "ERROR");
      }else{
        switch(chip[x].chipAddr[0])
        {
          case ds18b20ID:
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","T");
            break;
          }
   
          case ds2406ID:
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","S");
            break;
          }
          
          case t3tcID:
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","C");
            break;
          }

          case ds2762ID:
          case max31850ID:
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","K");
            break;
          }

          default:
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","Z");
            break;
          }
        }
      }
      sendUDPpacket();
      break;
    }
 
    case updateBonjour: // "9"
    {
      PacketBuffer[packetSize-1] = 0x00;
      sprintf(bonjourNameBuf, "%s", (char *) &PacketBuffer[1]);
      EthernetBonjour.setBonjourName(bonjourNameBuf);
      if(setDebug & bonjourDebug)
      {
        Serial.print(F("Bounjour Name set to: "));
        Serial.println(bonjourNameBuf);
      }
      EthernetBonjour.removeAllServiceRecords();
      if(setDebug & bonjourDebug)
      {
        Serial.println(F("Bounjour Service Records Removed"));
        Serial.print(F("Setting Bonjour Service record to "));
      }
      sprintf(bonjourBuf, "%s._discover", bonjourNameBuf);
      if(setDebug & bonjourDebug)
      {
        Serial.println(bonjourBuf);
      }
      EthernetBonjour.addServiceRecord(bonjourBuf, localPort, MDNSServiceUDP);
      
      I2CEEPROM_writeAnything(I2CEEPROMbjAddr, bonjourNameBuf, I2C0x50);
      if(setDebug & bonjourDebug)
      {
        Serial.print(F("Saving "));
        Serial.print(bonjourNameBuf);
        Serial.println(F(" to I2CEEPROM"));
      }
      lcd[7]->setCursor(0, 0);
      lcdCenterStr(bonjourNameBuf);
      lcd[7]->print(lcdStr);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "Service Record set to %s", bonjourBuf);
      sendUDPpacket();
      break;
    }
    

    case getActionArray: // "A"
    {
      if(setDebug & udpDebug)
      {
        Serial.println(F("getActionArray"));
      }
      x = atoi((char *) &PacketBuffer[1]);
      if(x >= maxActions)
      {
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "ERROR");
      }else{
        if(setDebug & udpDebug)
        {
          Serial.print(F("x = "));
          Serial.println(x);
        }
  
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",action[x].actionEnabled);
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s"," ");
        if(action[x].tempPtr == NULL)
        {
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",charChipAddrArray);
        }else{
          showChipAddress((uint8_t *) &action[x].tempPtr->chipAddr);
        }
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s"," ");
        if(action[x].tempPtr == NULL)
        {
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", unassignedStr);
        }else{
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", action[x].tempPtr->chipName);
        }
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s"," ");
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",action[x].tooCold);
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s"," ");
        if(action[x].tcPtr == NULL)
        {
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",charChipAddrArray);
        }else{
          showChipAddress((uint8_t *) &action[x].tcPtr->chipAddr);
        }
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s"," ");
        if(action[x].tcPtr == NULL)
        {
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", unassignedStr);
        }else{
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", action[x].tcPtr->chipName);
        }
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s"," ");
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c",(char) action[x].tcSwitchLastState);
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s"," ");
        itoa((action[x].tcDelay / 1000), itoaBuf, 10); 
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", itoaBuf);
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s"," ");
        itoa(action[x].tcMillis, itoaBuf, 10); 
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", itoaBuf);
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s"," ");
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",action[x].tooHot);
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s"," ");
        if(action[x].thPtr == NULL)
        {
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",charChipAddrArray);
        }else{
          showChipAddress((uint8_t *) &action[x].thPtr->chipAddr);
        }
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s"," ");
        if(action[x].thPtr == NULL)
        {
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", unassignedStr);
        }else{
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", action[x].thPtr->chipName);
        }
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s"," ");
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c",(char) action[x].thSwitchLastState);
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s"," ");
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%ld",(action[x].thDelay / 1000));
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s"," ");
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%ld",action[x].thMillis);
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s"," ");
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",action[x].lcdAddr);
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","\n");
      }
      sendUDPpacket();
    }
    break;
    
    case updateActionArray: // "B"
    {
      if(setDebug & actionDebug)
      {
        Serial.println(PacketBuffer);
      }
     
      result = strtok( PacketBuffer, delim );

      while(1)
      {
        if(setDebug & actionDebug)
        {
          Serial.print(F("resultCnt = "));
          Serial.println(resultCnt);
        }
        
        result = strtok( NULL, delim );
        
        if(setDebug & actionDebug)
        {
          Serial.print(F("result = "));
          Serial.println(result);
        }
        
        if(result == NULL){break;}
        
        switch (resultCnt)
        {
          case 0: // action
          {
            actionArray = atoi(result);
            if(setDebug & actionDebug)
            {
              Serial.print(F("Case 0: actionArray = "));
              Serial.println(actionArray);
            }
            break;
          }
          case 1:
          {
            actionSection = atoi(result);
            if(setDebug & actionDebug)
            {
              Serial.print(F("Case 1: actionSection = "));
              Serial.println(actionSection);
            }
            break;
          }
          
          case 2:
          {
            actionEnableTemp = atoi(result);
            
            if(setDebug & actionDebug)
            {
              Serial.print(F("Case 2: actionEnable = "));
              Serial.println(actionEnableTemp);
              Serial.print(F("action["));
              Serial.print(actionArray);
              Serial.print(F("]"));
            }
            
            switch (actionSection)
            {
              case 1:
              {
                if(actionEnableTemp == 1)
                {
                  action[actionArray].actionEnabled = TRUE;
                  
                  if(setDebug & actionDebug)
                  {
                    Serial.println(F(".actionEnabled is Enabled"));
                  }
                  
                }else{
                  action[actionArray].actionEnabled = FALSE;
                  
                  if(setDebug & actionDebug)
                  {
                    Serial.println(F(".actionEnabled is Disabled"));
                  }
                }
                
              if(setDebug & actionDebug)
                {
                  Serial.print(F("action["));
                  Serial.print(actionArray);
                  Serial.print(F("].actionEnabled = "));
                  Serial.println(action[actionArray].actionEnabled);
                }
                
                break;
              }
              
              case 2:
              case 3:
              {
                if(actionSection == 2)
                {
                  action[actionArray].tooCold = actionEnableTemp;
                  
                  if(setDebug & actionDebug)
                  {
                    Serial.print(F(".tooCold is set to "));
                    Serial.println(actionEnableTemp);
                  }
                  
                }else if( actionSection == 3){
                  action[actionArray].tooHot = actionEnableTemp;
                  
                  if(setDebug & actionDebug)
                  {
                    Serial.print(F(".tooHot is set to "));
                    Serial.println(actionEnableTemp);
                  }
                }
                break;
              }
            }
            break;
          }
          
          case 3:
          {
            if(actionSection != 1)
            {
              if(setDebug & actionDebug)
              {
                Serial.print(F("Case 3: result = "));
                Serial.println(result);
              }
              actionDelayVal = ((uint32_t) atoi(result));
              
              if(setDebug & actionDebug)
              {
                Serial.print(F("actionDelayVal = "));
                Serial.println(actionDelayVal);
              }
              
              actionDelayVal *= 1000;
              
              if(setDebug & actionDebug)
              {
                Serial.print(F("actionDelayVal * 1000 = "));
                Serial.println(actionDelayVal);
                Serial.print(F("action["));
                Serial.print(actionArray);
                Serial.print(F("]."));
              }
              
              if(actionSection == 2)
              {
                action[actionArray].tcDelay = actionDelayVal;
                if(actionDelayVal > 0)
                {
                  action[actionArray].tcMillis = millis();
                }
                
                if(setDebug & actionDebug)
                {
                  Serial.print(F("tcDelay = "));
                  Serial.println((actionDelayVal / 1000));
                }
                
              }else if (actionSection == 3){
                action[actionArray].thDelay = actionDelayVal;
                if(actionDelayVal > 0)
                {
                  action[actionArray].thMillis = millis();
                }
                
                if(setDebug & actionDebug)
                {
                  Serial.print(F("thDelay = "));
                  Serial.println(actionDelayVal / 1000);
                }
              }
            }
            break;
          }
          
          case 5:
          {
            if(setDebug & actionDebug)
            {
              Serial.print(F("Case 5 addrResult = "));
              Serial.println(result);
            }
            addrResult = strtok( result, addrDelim );
            while(addrResult != NULL)
            {
              addrVal[addrResultCnt] = (uint8_t) strtol(addrResult, NULL, 16);
              
              if(setDebug & actionDebug)
              {
                Serial.print(F(" "));
                Serial.print(addrVal[addrResultCnt], HEX);
              }
              
              addrResultCnt++;
              addrResult = strtok( NULL, addrDelim );
            }
            
            if(addrVal[0] == 0x0)
            {
              chipAddrCnt = maxChips+10;
            }else{
              chipAddrCnt = matchChipAddress(addrVal);
            }
            
            if(setDebug & actionDebug)
            {
              Serial.println();
              Serial.print(F("chipAddrCnt =  "));
              Serial.println(chipAddrCnt, HEX);
            }
            
            switch (actionSection)
            {
              case 1:
              {
                if(chipAddrCnt > chipCnt)
                {
                  action[actionArray].tempPtr = NULL;
                }else{
                  action[actionArray].tempPtr = &chip[chipAddrCnt];
                }
                break;
              }
              case 2:
              {
                if(chipAddrCnt > chipCnt)
                {
                  action[actionArray].tcPtr = NULL;
                }else{
                  action[actionArray].tcPtr = &chip[chipAddrCnt];
                }
                break;
              }
              case 3:
              {
                if(chipAddrCnt > chipCnt)
                {
                  action[actionArray].thPtr = NULL;
                }else{
                  action[actionArray].thPtr = &chip[chipAddrCnt];
                }
                break;
              }
            }
            break;
          }
          
          case 4:
          {
            if(actionSection == 1)
            {
              if(setDebug & actionDebug)
              {
                Serial.print(F("Case 4 LCD = "));
                Serial.println(result);
              }
              action[actionArray].lcdAddr = atoi(result);
              if(setDebug & actionDebug)
              {
                Serial.print(F("action["));
                Serial.print(actionArray);
                Serial.print(F("].lcdAddr = "));
                Serial.println(action[actionArray].lcdAddr);
              }
              if( (action[actionArray].lcdAddr >= 32 ) &&
                  (action[actionArray].lcdAddr <= 38 )
                )
              {
                action[actionArray].lcdMillis = millis();
              }else{
                action[actionArray].lcdMillis = 0;
              }
            }
            break;
          }
          break;
        }
        
        resultCnt++;
      }
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","OK");
      sendUDPpacket();
      break;
    }
      
    case getActionStatus: // "C"
    {
      getAllActionStatus();
      sendUDPpacket();
      break;
    }
    
    case getMaxActions: // "D"
    {
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",maxActions);
      sendUDPpacket();
    }
    break; 

    case setActionSwitch: // "E"
    {
      actionSelected = atoi((char *) &PacketBuffer[1]);
     if(actionSelected >= maxActions)
      {
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "ERROR");
      }else{
        if(actionSelected >= 10)
        {
          ssBufOffset = 3;
        }else{
          ssBufOffset = 2;
        }
        
        if(PacketBuffer[ssBufOffset+1] == setSwitchON)
        {
          setChipState = ds2406PIOAon;
        }else{
          setChipState = ds2406PIOAoff;
        }
        
        switch (PacketBuffer[ssBufOffset])
        {
          case tooColdSwitch:
          {
            actionSwitchSet((uint8_t *) action[actionSelected].tcPtr->chipAddr, setChipState);
            if(setChipState == ds2406PIOAoff && action[actionSelected].tcDelay > 0)
            {
              action[actionSelected].tcMillis = millis();
            }
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c",(char) action[actionSelected].tcPtr->chipStatus);
            break;
          }
          
          case tooHotSwitch:
          {
            actionSwitchSet((uint8_t *) action[actionSelected].thPtr->chipAddr, setChipState);
            if(setChipState == ds2406PIOAoff && action[actionSelected].thDelay > 0)
            {
              action[actionSelected].thMillis = millis();
            }
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c",(char) action[actionSelected].thPtr->chipStatus);
            break;
          }
          
          default:
          {
            break;
          }
        }
      }
      sendUDPpacket();
      break;
    }

    case saveToEEPROM: // "F"
    {
      saveStructures();
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","EEPROM Saved");
      sendUDPpacket();
      break;
    }
    
    case getEEPROMstatus: // "G"
    {
      if(i2cEepromReady == FALSE)
      {
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","FALSE");
      }else{
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","TRUE");
      }
     sendUDPpacket();
     break;
    }
    
    case getNewSensors: // "H"
    {
      // turn off all switches and clear chip structures
      for(x=0; x<maxChips; x++)
      {
        setSwitch(x, ds2406PIOAoff);
        for(int cAcnt = 0;cAcnt < 8; cAcnt++)
        {
          chip[x].chipAddr[cAcnt] = 0x00;
        }
        chip[x].chipStatus = 0;
        chip[x].tempTimer = 0;
      }
      
      // disable and clear actions
      for(x=0; x<maxActions; x++)
      {
        action[x].actionEnabled = FALSE;
        action[x].tempPtr = NULL;
        action[x].tooCold = -255;
        action[x].tcPtr = NULL;
        action[x].tcSwitchLastState = 'F';
        action[x].tcDelay = 0;
        action[x].tcMillis = 0;
        action[x].tooHot = 255;
        action[x].thPtr = NULL;
        action[x].thSwitchLastState = 'F';
        action[x].thDelay = 0;
        action[x].thMillis = 0;
      }

      // disable and clear pid
      for(x=0; x<maxPIDs; x++)
      {
        ePID[x].pidEnabled = FALSE;
        ePID[x].tempPtr = NULL;
        ePID[x].pidSetPoint = 70;
        ePID[x].switchPtr = NULL;
        ePID[x].pidKp = 0;
        ePID[x].pidKi = 0;
        ePID[x].pidKd = 0;
        ePID[x].pidDirection = 0;
        ePID[x].pidwindowStartTime = 5000;
        ePID[x].pidInput = 0;
        ePID[x].pidOutput = 0;
        ePID[x].myPID = NULL;
      }
      
      saveStructures();

      // find new chips
      findChips();
      
      saveStructures();
      
      readStructures();
      
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d Chips Found, getNewSensors Complete", chipCnt);
      sendUDPpacket();
      break;
    }
    
    case masterStop: //"I"
    {
      MasterStop();
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","masterStop Complete");
      sendUDPpacket();
      break;
    }
    
    case getMaxPids: // "J"
    {
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",maxPIDs);
      sendUDPpacket();
      break;
    }
    
    case masterPidStop: // "K"
    {
      if(setDebug & pidDebug)
      {
        Serial.println(F("masterPidStop Enter"));
      }
      
      for(x=0;x<maxPIDs;x++)
      {
        ePID[x].pidEnabled = FALSE;
        
        if(setDebug & pidDebug)
        {
          Serial.print(F("ePID["));
          Serial.print(x);
          Serial.println(F("].pidEnabled set to FALSE"));
        }
        
        ePID[x].myPID->SetMode(MANUAL);
        
        if(setDebug & pidDebug)
        {
          Serial.print(F("ePID["));
          Serial.print(x);
          Serial.println(F("].myPID->SetMode() set to MANUAL"));
        }
        
        if(&ePID[x].switchPtr->chipAddr != NULL)
        {
          actionSwitchSet((uint8_t *) &ePID[x].switchPtr->chipAddr, ds2406PIOAoff);
          if(setDebug & pidDebug)
          {
              Serial.print(F("ePID["));
              Serial.print(x);
              Serial.println(F("].switchPtr->chipAddr set to OFF"));
          }
        }
      }
      
      if(setDebug & pidDebug)
      {
        Serial.println(F("masterPidStop Exit"));
      }
      
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","masterPIDStop Completed");
      sendUDPpacket();
      break;
    }
    
    case getPidStatus: // "L"
    {
      for(int x = 0; x < maxPIDs; x++)
      {
        getPIDStatus(x);
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", ';');
      }
      sendUDPpacket();
      break;
    }

    case updatePidArray: // "M"
    {
      if(setDebug & pidDebug)
      {
        Serial.println(F("updatePidArray Enter"));
        Serial.println(PacketBuffer);
      }
      
      result = strtok( PacketBuffer, delim );
      char* pidArrayPtr      = strtok( NULL, delim );
      char* pidEnabledPtr    = strtok( NULL, delim );
      char* pidTempAddrPtr   = strtok( NULL, delim );
      char* pidSetPointPtr   = strtok( NULL, delim );
      char* pidSwitchAddrPtr = strtok( NULL, delim );
      char* pidKpPtr         = strtok( NULL, delim );
      char* pidKiPtr         = strtok( NULL, delim );
      char* pidKdPtr         = strtok( NULL, delim );
      char* pidDirectionPtr  = strtok( NULL, delim );
      char* pidWindowSizePtr = strtok( NULL, delim );
      
      if(strlen(pidTempAddrPtr) < 39 || strlen(pidSwitchAddrPtr) < 39)
      {
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","Z");
        break;
      }

      pidArray = atoi(pidArrayPtr);
      if(setDebug & pidDebug)
      {
        Serial.print(F("pidArray = "));
        Serial.println(pidArray);
        Serial.print(F("pidEnabledPtr = "));
        Serial.println(pidEnabledPtr);
        Serial.print(F("pidTempAddrPtr = "));
        Serial.println(pidTempAddrPtr);
        Serial.print(F("pidSetPointPtr = "));
        Serial.println(pidSetPointPtr);
        Serial.print(F("pidSwitchAddrPtr = "));
        Serial.println(pidSwitchAddrPtr);
        Serial.print(F("pidKpPtr = "));
        Serial.println(pidKpPtr);
        Serial.print(F("pidKiPtr = "));
        Serial.println(pidKiPtr);
        Serial.print(F("pidKdPtr = "));
        Serial.println(pidKdPtr);
        Serial.print(F("pidDirectionPtr = "));
        Serial.println(pidDirectionPtr);
        Serial.print(F("pidWindowSizePtr = "));
        Serial.println(pidWindowSizePtr);
      }
  
      if(setDebug & pidDebug)
      {
        Serial.print(F("pidEnabled = "));
        Serial.println(ePID[pidArray].pidEnabled);
      }
      
      asciiArrayToHexArray(pidTempAddrPtr, addrDelim, addrVal);
      
      if(addrVal[0] == 0x0)
      {
        chipAddrCnt = maxChips + 10;
      }else{
        chipAddrCnt = matchChipAddress(addrVal);
      }
      
      if(setDebug & pidDebug)
      {
        Serial.print(F("chipAddrCnt = "));
        Serial.println(chipAddrCnt);
      }
      
      if(chipAddrCnt > chipCnt)
      {
        ePID[pidArray].tempPtr = NULL;
      }else{
        ePID[pidArray].tempPtr = &chip[chipAddrCnt];
      }
      
      if(setDebug & pidDebug)
      {
        Serial.print(F("tempPtr = "));
        Serial.println((uint32_t) ePID[pidArray].tempPtr, HEX);
      }

      ePID[pidArray].pidSetPoint = strtod(pidSetPointPtr, NULL);
      
      if(setDebug & pidDebug)
      {
        Serial.print(F("pidSetPoint = "));
        Serial.println((double) ePID[pidArray].pidSetPoint);
      }
  
      asciiArrayToHexArray(pidSwitchAddrPtr, addrDelim, addrVal);
      
      if(addrVal[0] == 0x0)
      {
        chipAddrCnt = maxChips + 10;
      }else{
        chipAddrCnt = matchChipAddress(addrVal);
      }
      
      if(setDebug & pidDebug)
      {
        Serial.print(F("chipAddrCnt = "));
        Serial.println(chipAddrCnt);
      }
      
      if(chipAddrCnt > chipCnt)
      {
        ePID[pidArray].switchPtr = NULL;
      }else{
        ePID[pidArray].switchPtr = &chip[chipAddrCnt];
      }
      
      if(setDebug & pidDebug)
      {
        Serial.print(F("switchPtr = "));
        Serial.println((uint32_t) ePID[pidArray].switchPtr, HEX);
      }

      ePID[pidArray].pidKp = strtod(pidKpPtr, NULL);
      
      if(setDebug & pidDebug)
      {
        Serial.print(F("pidKp = "));
        Serial.println((double) ePID[pidArray].pidKp);
      }

      ePID[pidArray].pidKi = strtod(pidKiPtr, NULL);
      
      if(setDebug & pidDebug)
      {
        Serial.print(F("pidKi = "));
        Serial.println((double) ePID[pidArray].pidKi);
      }

      ePID[pidArray].pidKd = strtod(pidKdPtr, NULL);
      
      if(setDebug & pidDebug)
      {
        Serial.print(F("pidKd = "));
        Serial.println((double) ePID[pidArray].pidKd);
      }
      
      ePID[pidArray].myPID->SetTunings(ePID[pidArray].pidKp, ePID[pidArray].pidKi, ePID[pidArray].pidKd);

      pidDirectionVal = atoi(pidDirectionPtr);
      if(pidDirectionVal == 0)
      {
        ePID[pidArray].pidDirection = FALSE;
      }else{
        ePID[pidArray].pidDirection = TRUE;
      }
      
      if(setDebug & pidDebug)
      {
        Serial.print(F("pidDirection = "));
        Serial.println(ePID[pidArray].pidDirection);
      }
      
      ePID[pidArray].myPID->SetControllerDirection(ePID[pidArray].pidDirection);

      ePID[pidArray].pidWindowSize = strtol(pidWindowSizePtr, NULL, 10);
      ePID[pidArray].myPID->SetOutputLimits(0, ePID[pidArray].pidWindowSize);
      ePID[pidArray].pidwindowStartTime = millis();

      pidEnabledVal = atoi(pidEnabledPtr);
      if( (pidEnabledVal == 0) ||
          (ePID[pidArray].pidKp == 0) ||
          (ePID[pidArray].pidKi == 0) ||
          (ePID[pidArray].pidKd == 0) ||
          (ePID[pidArray].pidWindowSize == 0) ||
          (ePID[pidArray].switchPtr == NULL) ||
          (ePID[pidArray].tempPtr == NULL)
        )
      {
        ePID[pidArray].pidEnabled = FALSE;
        ePID[pidArray].myPID->SetMode(MANUAL);
      }else{
        ePID[pidArray].pidEnabled = TRUE;
        ePID[pidArray].myPID->SetMode(AUTOMATIC);
      }

      if(setDebug & pidDebug)
      {
        Serial.print(F("pidEnabled = "));
        Serial.println(ePID[pidArray].pidEnabled);
      }
      
      if(setDebug & pidDebug)
      {
        Serial.print(F("pidWindowSize = "));
        Serial.println((double) ePID[pidArray].pidWindowSize);
      }
     rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","OK");
     sendUDPpacket();
     break;
    }


    case getPidArray: // "N"
    {
      x = atoi((char *) &PacketBuffer[1]);
      if(x >= maxPIDs)
      {
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","ERROR");
      }else{
        getPIDStatus(x);
      }
      sendUDPpacket();
      break;
    }
    
    case setPidArray: // "O"
    {
      result = strtok( PacketBuffer, delim );
      char* pidArrayCtr      = strtok( NULL, delim );
      char* pidEnabledVal    = strtok( NULL, delim );
      
      x = atoi(pidArrayCtr);
      if (atoi(pidEnabledVal) == 0)
      {
        ePID[x].pidEnabled = FALSE;
        ePID[x].myPID->SetMode(MANUAL);
        actionSwitchSet((uint8_t *) &ePID[x].switchPtr->chipAddr, ds2406PIOAoff);
      }else{
        ePID[x].pidEnabled = TRUE;
        ePID[x].pidwindowStartTime = millis();
        ePID[x].myPID->SetMode(AUTOMATIC);
      }
      sendUDPpacket();
      break;
    }
    
    case useDebug: // "P"
    {
      setDebug = 0x00000000;
      char *pEnd;
      setDebug |= strtol((char *) &PacketBuffer[1], &pEnd, 16);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","0x");
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%08lX",setDebug);
      sendUDPpacket();
      break;
    }
 
    case restoreStructures: // "Q"
    {
      readStructures();
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","EEPROMread");
      sendUDPpacket();
      break;
    }
    
    case shortShowChip: // "R"
    {
      x = atoi((char *) &PacketBuffer[1]);
      if( x >= maxChips)
      {
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","ERROR");
      }else{
        showChipType(x);
      }
      sendUDPpacket();
      break;
    }
    
    case updateChipName: // "S"
    {
      uint8_t cnCnt;
      
      if(setDebug & chipNameDebug)
      {
        Serial.println(F("updateChipName Enter"));
        Serial.println(PacketBuffer);
      }
      
      result = strtok( PacketBuffer, delim );
      char* chipNameAddr      = strtok( NULL, delim );
      char* chipNameStr       = strtok( NULL, delim ); 
     
      asciiArrayToHexArray(chipNameAddr, addrDelim, addrVal);
      chipAddrCnt = matchChipAddress(addrVal);
      
      for(cnCnt = 0; cnCnt < chipNameSize; cnCnt++)  // clear the name array
      {
        chip[chipAddrCnt].chipName[cnCnt] = 0x00;
      }
      
      strcpy(chip[chipAddrCnt].chipName, chipNameStr); //copy the name array

      for(cnCnt = 0; cnCnt < chipNameSize; cnCnt++)
      {
        if(chip[chipAddrCnt].chipName[cnCnt] < 0x30 || chip[chipAddrCnt].chipName[cnCnt] > 0x7A)
        {
          chip[chipAddrCnt].chipName[cnCnt] = 0x00; //remove non-ascii characters
        }
      }

      if(setDebug & chipNameDebug)
      {
        Serial.print(F("chip["));
        Serial.print(chipAddrCnt);
        Serial.print(F("].chipName = "));
        Serial.println(chip[chipAddrCnt].chipName);
        for(cnCnt = 0; cnCnt < chipNameSize; cnCnt++)
        {
          Serial.print(F("0x"));
          if(chip[chipAddrCnt].chipName[cnCnt] < 0x0f)
          {
            Serial.print(F("0"));
          }
          Serial.print(chip[chipAddrCnt].chipName[cnCnt], HEX);
          if(cnCnt < chipNameSize - 1)
          {
            Serial.print(F(", "));
          }
        }
        Serial.println();
      }
      
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","Name Updated");
      sendUDPpacket();
      break;
    }
    
    case showActionStatus:  // "T"
    {
      x = atoi((char *) &PacketBuffer[1]);
      if( x >= maxActions )
      {
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","ERROR");
      }else{
        actionStatus(x);
      }
      sendUDPpacket();
      break;
    }
    
    case setAction: // "U"
    {
      result = strtok( PacketBuffer, delim );
      char* actionArrayCtr      = strtok( NULL, delim );
      char* actionEnabledVal    = strtok( NULL, delim );
      
      x = atoi(actionArrayCtr);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "Action %d ", x);
      if (atoi(actionEnabledVal) == 0)
      {
        setChipState = ds2406PIOAoff;
        actionSwitchSet((uint8_t *) action[x].tcPtr->chipAddr, setChipState);
        actionSwitchSet((uint8_t *) action[x].thPtr->chipAddr, setChipState);
        action[x].actionEnabled = FALSE;
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "DISABLED");
      }else{
        action[x].actionEnabled = TRUE;
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "ENABLED");
      }
      sendUDPpacket();
      break;
    }

    case displayMessage: // "w"
    {
      /*********************
      Display LCD String format is:
      
      lcdDisplayStr lcdLineCtr clrLcdStr lcdMessage
      
      where:
      lcdDisplayStr is the i2c address of the display, currently 0 - 7
      lcdLineCtr is the line on which to put the message 0 - 4
      clrLcdStr clears the entire display
      lcdMessage is the message string, currently 20 characters, more than 20 results in an error
      
      underscores are converted to spaces, line and carriage returns are zeroed out. 
      
      *********************/
      result = strtok( PacketBuffer, delim );

      char* lcdDisplayStr  = strtok( NULL, delim );
      char* lcdLineCtr     = strtok( NULL, delim );
      char* clrLcdStr      = strtok( NULL, delim );
      char* lcdMessage     = strtok( NULL, delim );
      
      uint8_t lcdDisplay = atoi(lcdDisplayStr);
      uint8_t lcdLine = atoi(lcdLineCtr);
      uint8_t clrLCD = atoi(clrLcdStr);
      
      uint8_t msgLength = strlen(lcdMessage);
      
      if(setDebug & lcdDebug)
      {
        Serial.print(F("lcdMessage is "));
        Serial.println(lcdMessage);
        for( int t = 0; t < msgLength; t++)
        {
          Serial.print(F("0x"));
          if(lcdMessage[t] < 0x10)
          {
            Serial.print(F("0"));
          }
          Serial.print(lcdMessage[t], HEX);
          if(t < (msgLength - 1))
          {
            Serial.print(F(", "));
          }
        }
        Serial.println();
      }
      
      if( msgLength > 21 )
      {
        if(setDebug & lcdDebug)
        {
          Serial.print(F("lcdMessage is "));
          Serial.print(msgLength);
          Serial.println(F(" too long, truncating"));
        }
        lcdMessage[20] = 0x0;
      }
      
      if(clrLCD == 1)
      {
        lcd[lcdDisplay]->clear();
        lcd[lcdDisplay]->home();
      }
      
      lcd[lcdDisplay]->setCursor(0, lcdLine);
      for(int s = 0; s < lcdChars; s++)
      {
        switch(lcdMessage[s])
        {
          case '_':
          {
            lcdMessage[s] = ' ';
            break;
          }
          
          case 0x0A:
          case 0x0D:
          {
            lcdMessage[s] = 0x00;            
            break;
          }
        }
      }
      lcd[lcdDisplay]->print(lcdMessage);

      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "OK");
      sendUDPpacket();
      break;
    }
    
    case clearAndReset: // "x"
    {
      int x;
      MasterStop();
      if(setDebug & resetDebug)
      {
        Serial.println(F("MasterStop() completed"));
        delay(1000);
      }
      
      EEPROMclear();
      if(setDebug & resetDebug)
      {
        Serial.println(F("EEPROMclear() completed"));
      }
      I2CEEPROM_readAnything(I2CEEPROMidAddr, i2cEeResult, I2C0x50);
  
      if(setDebug & resetDebug)
      { 
        Serial.print(F("i2cEeResult = 0x"));
        Serial.println(i2cEeResult, HEX);
      }
  
      if(i2cEeResult != 0x55)
      {
        if(setDebug & resetDebug)
        { 
          Serial.println(F("No EEPROM Data"));
          delay(1000);
        }
      }
        
      for(x = 0; x < maxChips; x++)
      {
        memcpy(&chip[x], &chipClear, sizeof(chipStruct));
      }
      
      if(setDebug & resetDebug)
      { 
        Serial.println(F("chip structures cleared"));
        delay(1000);
      }
  
      for(x = 0; x < maxActions; x++)
      {
        memcpy(&action[x], &actionClear, sizeof(chipActionStruct));
      }
      
      if(setDebug & resetDebug)
      { 
        Serial.println(F("action structures cleared"));
        delay(1000);
      }
  
      for(x = 0; x < maxPIDs; x++)
      {
        memcpy(&ePID[x], &pidClear, sizeof(chipPIDStruct));
      }
      
      if(setDebug & resetDebug)
      { 
        Serial.println(F("pid structures cleared"));
        delay(1000);
      }
  
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","Resetting");
      sendUDPpacket();
      delay(1000);
      softReset();
      break;
    }
    
    case clearEEPROM: // "y"
    {
      EEPROMclear();
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","EEPROM Cleared");
      sendUDPpacket();
      break;
    }
    
    case versionID: // "z"
    {
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", versionStrName);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",", ");
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", teensyType);      
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", versionStrNumber);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",", ");
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", versionStrDate);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",", ");
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", bonjourNameBuf);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",", IP Address = ");
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d", Ethernet.localIP()[0]);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", '.');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d", Ethernet.localIP()[1]);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", '.');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d", Ethernet.localIP()[2]);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", '.');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d", Ethernet.localIP()[3]);
      sendUDPpacket();
      break;
    }
    default:
    {
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","Invalid Command");
      sendUDPpacket();
      break;
    }   
  }
  
  PacketBuffer[0]=0x00;
  cnt = 0;
  serialMessageReady = FALSE;
  udpTimer = 0; // reset udp watchdog
}

void asciiArrayToHexArray(char* result, char* addrDelim, uint8_t* addrVal)
{
  char *addrResult = NULL;
  uint16_t addrResultCnt = 0;
  
  addrResult = strtok( result, addrDelim );
  while(addrResult != NULL)
  {
    addrVal[addrResultCnt] = (uint8_t) strtol(addrResult, NULL, 16);
    
    if(setDebug & pidDebug)
    {
      if(addrVal[addrResultCnt] >= 0 && addrVal[addrResultCnt] <= 9)
      {
        Serial.print(F(" 0x0"));
      }else{
        Serial.print(F(" 0x"));
      }
      Serial.print(addrVal[addrResultCnt], HEX);
    }
      
    addrResultCnt++;
    addrResult = strtok( NULL, addrDelim );
   }
   
  if(setDebug & pidDebug)
  {
    Serial.println();
  }
}


void actionStatus(int x)
{
  rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",(int) action[x].actionEnabled);
  rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",",");
  if(action[x].tempPtr == NULL)
  {
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","NULL");
  }else{
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",(int) action[x].tempPtr->chipStatus);
  }
    
  rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",",");
    
  if(action[x].tcPtr == NULL)
  {
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","NULL");
  }else{
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c",(char) action[x].tcPtr->chipStatus);
  }

  rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",",");

  if(action[x].thPtr == NULL)
  {
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","NULL");
  }else{
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c",(char) action[x].thPtr->chipStatus);
  }   
  rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",",");
  rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",(int) action[x].tooCold);
  rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",",");
  rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",(int) action[x].tooHot);
}

void getAllActionStatus(void)
{
  for( int x = 0; x < maxActions; x++ )
  {
    actionStatus(x);
    if( x < (maxActions - 1) )
    {
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",";");
    }
  }
}

void getPIDStatus(uint8_t x)
{ 
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d", (int) ePID[x].pidEnabled);
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", ' ');
    if(ePID[x].tempPtr == NULL)
    {
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", charChipAddrArray);      
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c" ,' ');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", unassignedStr);      
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c" ,' ');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",(int) noChipPresent);
    }else{
      showChipAddress((uint8_t *) ePID[x].tempPtr->chipAddr);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c" ,' ');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", ePID[x].tempPtr->chipName);      
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c" ,' ');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",(int) ePID[x].tempPtr->chipStatus);
    }
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", ' ');
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",(int) ePID[x].pidSetPoint);
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", ' ');
    if(ePID[x].switchPtr == NULL)
    {
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", charChipAddrArray);      
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c" ,' ');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", unassignedStr);      
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c" ,' ');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",(int) noChipPresent);
    }else{
      showChipAddress((uint8_t *) ePID[x].switchPtr->chipAddr);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c" ,' ');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", ePID[x].switchPtr->chipName);      
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c" ,' ');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c",(char) ePID[x].switchPtr->chipStatus);
    }
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", ' ');
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%.2f",(double) ePID[x].pidKp);
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", ' ');
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%.2f",(double) ePID[x].pidKi);
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", ' ');
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%.2f",(double) ePID[x].pidKd);
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", ' ');
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",(int) ePID[x].pidDirection);
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", ' ');
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%ld",(uint32_t) ePID[x].pidWindowSize);
}

uint8_t matchChipAddress(uint8_t* array)
{
   uint8_t addrMatchCnt, chipAddrCnt;
   
  if(setDebug & pidDebug)
  {
   Serial.println(F("matchChipAddress"));
  }
  
  for(addrMatchCnt = 0, chipAddrCnt = 0; ((addrMatchCnt < chipAddrSize) || (chipAddrCnt > chipCnt)); addrMatchCnt++)
  {
    if(array[addrMatchCnt] != chip[chipAddrCnt].chipAddr[addrMatchCnt])
    {
      addrMatchCnt = 0;
      chipAddrCnt++;
      
      if(setDebug & pidDebug)
      {
        Serial.println(chipAddrCnt);
      }
  
      continue;
    }
    
    if(setDebug & pidDebug)
    {
      Serial.print(array[addrMatchCnt], HEX);
      Serial.print(F(","));
    }
  }
  
  if(chipAddrCnt <= chipCnt)
  {
    if(setDebug & pidDebug)
    {
      Serial.print(F("MATCH!! - "));
    }
  }else{

    if(setDebug & pidDebug)
    {
      Serial.print(F("NO MATCH!! - "));
    }

    chipAddrCnt = 0xFF;
  }

  if(setDebug & pidDebug)
  {
    Serial.println(chipAddrCnt);
  }

  return(chipAddrCnt);
}

void actionSwitchSet(uint8_t* array, uint8_t setChipState)
{
   uint8_t chipAddrCnt;

  chipAddrCnt = matchChipAddress(array);
  
  if(chipAddrCnt != 0xFF)
  {
    setSwitch(chipAddrCnt, setChipState);
  }
}

void showChipAddress( uint8_t* array)
{
  if(setDebug & findChipDebug)
  {
    Serial.print(F("Chip Address 0x"));
    Serial.print((uint32_t) array, HEX);
    Serial.print(F(" = "));
  }
  
  for( int i = 0; i < chipAddrSize; i++)
  {
    rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s", "0x");    
    rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%02X",(uint8_t) array[i]);
    if(setDebug & findChipDebug)
    {
      Serial.print(F("0x"));
      if( array[i] < 0x10 ) Serial.print(F("0")); 
      Serial.print((uint8_t) array[i], HEX);
      if(i < 7)Serial.print(F(","));
    }
    if(i < 7)
    {
      rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s",",");
    }
  }
  if(setDebug & findChipDebug)
  {
    Serial.println();
  }
  
}

void showChipType(int x)
{
  rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s", "0x");    
  rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%02X", chip[x].chipAddr[0]);
  rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s"," ");
  addChipStatus(x);
}

void showChipInfo(int x)
{
  showChipAddress((uint8_t *) &chip[x].chipAddr);
  rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s"," ");
  addChipStatus(x);
}

void addChipStatus(int x)
{
  switch(chip[x].chipAddr[0])
  {
    case ds18b20ID:
    case ds2762ID:
    case t3tcID:
    case max31850ID:
    {
      rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%d", (int16_t) chip[x].chipStatus);
      break;
    }
    case ds2406ID:
    {
      rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%c", (int8_t) chip[x].chipStatus);
      break;
    }
    default:
    {
      rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%c", 'Z');
      break;
    }
  }
  rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s"," ");
  if(chip[x].chipName[0] == 0x00)
  {
    rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s",unassignedStr);
  }else{
    rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s", chip[x].chipName);
  }
}

void setSwitch(uint8_t x, uint8_t setChipState)
{
  if(chip[x].chipAddr[0] == 0x12)
  {
    ds.reset();
    ds.select(chip[x].chipAddr);
    ds.write(ds2406MemWr);
    ds.write(ds2406AddLow);
    ds.write(ds2406AddHi);
    ds.write(setChipState);
    for ( int i = 0; i < 6; i++)
    {
      chipBuffer[i] = ds.read();
    }
    ds.write(ds2406End);
    ds.reset();
    updateChipStatus(x);
  }
}

void updateChipStatus(int x)
{
  uint16_t chipCRCval, chipBufferCRC, noCRCmatch =1;
  
  digitalWrite(chipStartPin, LOW); //start DSO sync

  switch(chip[x].chipAddr[0])
  {
    
    case ds2762ID:
    {
      if(millis() >= chip[x].tempTimer + ds2762UpdateTime)
      {
        if(setDebug & ds2762Debug)
        {
          startTime = millis();
          Serial.println(F("Enter Read DS2762 Lookup"));
        }
        Read_TC_Volts(x);
        Read_CJ_Temp(x);
        cjComp = pgm_read_word_near(kTable + cjTemperature);
        if(setDebug & ds2762Debug)
        {
          Serial.print(F("kTable["));
          Serial.print(cjTemperature);
          Serial.print(F("] = "));
          Serial.println(pgm_read_word_near(kTable + cjTemperature));
        }
        if(sign == 1)
        {
          if(tcVoltage < cjComp)
          {
            cjComp -= tcVoltage;
          }else{
            cjComp = 0;
          }
        }else{
          cjComp += tcVoltage;
        }
        if(setDebug & ds2762Debug)
        {
          Serial.print(F("cjComp = "));
          Serial.print(cjComp);
          Serial.println(F(" microvolts"));
        }
        tblHi = kTableCnt - 1;
        TC_Lookup();
        if(error == 0)
        {
          if(setDebug & ds2762Debug)
          {
            Serial.print(F("Temp = "));
            Serial.print(eePntr);
            Serial.print(F(" degrees C, "));
            Serial.print(((eePntr * 9) / 5) + 32);
            Serial.println(F(" degrees F"));
          }
          if(showCelsius == TRUE)
          {
            chip[x].chipStatus = eePntr;
          }else{
            chip[x].chipStatus = ((eePntr * 9) / 5) + 32;
          }
        }else{
          if(setDebug & ds2762Debug)
          {
            Serial.println(F("Value Out Of Range"));
          }
        }
        if(setDebug & ds2762Debug)
        {
          endTime = millis();
          Serial.print(F("Exit Read DS2762 Lookup - "));
          Serial.print(endTime - startTime);
          Serial.println(F(" milliseconds"));
          Serial.println();
          Serial.println();
        }
        chip[x].tempTimer = millis() + tempReadDelay;
      }
      break;
    }
    
    case ds18b20ID:
    case t3tcID:
    case max31850ID:
    {
      if(chip[x].tempTimer == 0)
      {
        ds.reset();
        ds.select(chip[x].chipAddr);
        ds.write(0x4E); // write to scratchpad;
        ds.write(0x00); // low alarm
        ds.write(0x00); // high alarm
        ds.write(0x1F); // configuration register - 9 bit accuracy (0.5deg C)
        ds.reset();
        ds.select(chip[x].chipAddr);
        ds.write(0x44);         // start conversion
        chip[x].tempTimer = millis();
      }

      if((chip[x].tempTimer != 0) && (millis() >= chip[x].tempTimer + tempReadDelay))
      {
        ds.reset();
        ds.select(chip[x].chipAddr);    
        ds.write(0xBE);         // Read Scratchpad
  
        for (int i = 0; i < 9; i++) 
        {
          chipBuffer[i] = ds.read();
        }
        
        if(ds.crc8(chipBuffer, 8) != chipBuffer[8])
        {
          if(setDebug & crcDebug)
          {
            Serial.print(F("crc Error chip["));
            Serial.print(x);
            Serial.println(F("], resetting timer"));
          }
          chip[x].tempTimer = 0; // restart the chip times
          break; // CRC invalid, try later
        }
      // convert the data to actual temperature
        int raw = (chipBuffer[1] << 8) | chipBuffer[0];
        if( showCelsius == TRUE)
        {
          if(chip[x].chipAddr[0] == 0xAA)
          {
            chip[x].chipStatus = raw;
          }else{
            chip[x].chipStatus = (int16_t) ((float)raw / 16.0);
          }
        }else{
          if(chip[x].chipAddr[0] == 0xAA)
          {
            chip[x].chipStatus = (int16_t) ((((float)raw) * 1.8) + 32.0) ;
          }else{
            chip[x].chipStatus = (int16_t) ((((float)raw / 16.0) * 1.8) + 32.0);
          }
        }
        chip[x].tempTimer = 0;
      }
      break;
    }
    
    case ds2406ID:
    {
      while(noCRCmatch)
      {
        ds.reset();
        ds.select(chip[x].chipAddr);
        ds.write(ds2406MemRd);
        chipBuffer[0] = ds2406MemRd;
        ds.write(0x0); //2406 Addr Low
        chipBuffer[1] = 0;
        ds.write(0x0); //2406 Addr Hgh
        chipBuffer[2] = 0;
        for(int i = 3; i <  13; i++)
        {
          chipBuffer[i] = ds.read();
        }
        ds.reset();

        chipCRCval = ~(ds.crc16(chipBuffer, 11)) & 0xFFFF;
        chipBufferCRC = ((chipBuffer[12] << 8) | chipBuffer[11]) ;

        if(setDebug & chipDebug)
        {
          Serial.print(F("chip "));
          Serial.print(x);
          Serial.print(F(" chipCRC = 0X"));
          Serial.print(chipCRCval, HEX);
          Serial.print(F(", chipBufferCRC = 0X"));
          Serial.println(chipBufferCRC, HEX);
        }
        
        if(chipBufferCRC == chipCRCval) noCRCmatch = 0;
        
        if(chipBuffer[10] & dsPIO_A)
        {
          chip[x].chipStatus = switchStatusOFF;
        }else{
          chip[x].chipStatus = switchStatusON;
        }
      }
      break;
    }
    
    default:
    {
      chip[x].chipStatus = noChipPresent;
      break; 
    }
  }
  digitalWrite(chipStartPin, HIGH); //stop DSO sync
}

void updateActions(uint8_t x)
{
  uint8_t LCDx, y, tempStrCnt;
  char tempStr[6];
  uint32_t lcdUpdateStart, lcdUpdateStop;

  if(action[x].actionEnabled == TRUE)
  {
    if(action[x].tempPtr->chipStatus <= action[x].tooCold &&
       action[x].tcPtr->chipStatus == switchStatusOFF) // too cold
    {
      if(action[x].tcDelay == 0 || millis() > (action[x].tcMillis + action[x].tcDelay))
      {
        actionSwitchSet((uint8_t *) &action[x].tcPtr->chipAddr, ds2406PIOAon);
        
      }
    }else if(action[x].tempPtr->chipStatus > action[x].tooCold &&
             action[x].tcPtr->chipStatus == switchStatusON){
               
      actionSwitchSet((uint8_t *) &action[x].tcPtr->chipAddr, ds2406PIOAoff);
      action[x].tcMillis = millis();
       
    }

    if(action[x].tempPtr->chipStatus >= action[x].tooHot &&
       action[x].thPtr->chipStatus == switchStatusOFF) //too hot
    {
      if(action[x].thDelay == 0 || millis() > (action[x].thMillis + action[x].thDelay))
      {
        actionSwitchSet((uint8_t *) &action[x].thPtr->chipAddr, ds2406PIOAon);
        
      }
    }else if(action[x].tempPtr->chipStatus < action[x].tooHot &&
             action[x].thPtr->chipStatus == switchStatusON){
               
      actionSwitchSet((uint8_t *) &action[x].thPtr->chipAddr, ds2406PIOAoff);
      action[x].thMillis = millis();
               
    }
    
    if( (action[x].lcdAddr >= 32 ) &&
        (action[x].lcdAddr <= 38 ) &&
        ( (action[x].lcdMillis + lcdUpdateTimer) <= millis())
      )
    {
      if(setDebug & lcdDebug)
      {
        lcdUpdateStart = millis();
      }
      LCDx = (action[x].lcdAddr - 32);
//      lcd[LCDx]->clear();
      lcd[LCDx]->home();
      lcd[LCDx]->print(F("     Action #"));
      lcd[LCDx]->print(x);
      lcd[LCDx]->print(F("      "));
      lcd[LCDx]->setCursor(0, 1);
      tempStrCnt = strlen(action[x].tempPtr->chipName);
      if(setDebug & lcdDebug)
      {
        Serial.print(F("tempStrCnt for "));
        Serial.print(action[x].tempPtr->chipName);
        Serial.print(F(" is "));
        Serial.println(tempStrCnt);
      }
      lcd[LCDx]->print(action[x].tempPtr->chipName);
      lcd[LCDx]->setCursor(tempStrCnt, 1);
      for( y = (tempStrCnt); y < (lcdChars - 5); y++ )
      {
        lcd[LCDx]->print(F(" "));
      }
      tempStrCnt = sprintf( tempStr, "%d", (int16_t) action[x].tempPtr->chipStatus);
      if(setDebug & lcdDebug)
      {
        Serial.print(F("tempStrCnt for action["));
        Serial.print(x);
        Serial.print(F("]tempPtr.->chipStatus"));
        Serial.print(F(" is "));
        Serial.print(tempStrCnt);
        Serial.print(F(" and the chipStatus is "));
        Serial.println((int16_t) action[x].tempPtr->chipStatus);
     }

      switch(tempStrCnt)
      {
        case 1:
          {
            lcd[LCDx]->print(F("    "));            
            break;
          }
        case 2:
          {
            lcd[LCDx]->print(F("   "));            
            break;
          }
        case 3:
          {
            lcd[LCDx]->print(F("  "));            
            break;
          }
        case 4:
          {
            lcd[LCDx]->print(F(" "));            
            break;
          }
      }
      lcd[LCDx]->print((int16_t) action[x].tempPtr->chipStatus);
      lcd[LCDx]->setCursor(0, 2);
      if(action[x].tcPtr != NULL)
      {
        tempStrCnt = strlen(action[x].tcPtr->chipName);
      }else{
        tempStrCnt= strlen("NOT USED");
      }
      if(setDebug & lcdDebug)
      {
        if(action[x].tcPtr != NULL)
        {
          Serial.print(F("tempStrCnt for "));
          Serial.print(action[x].tcPtr->chipName);
          Serial.print(F(" is "));
          Serial.println(tempStrCnt);
        }else{
          Serial.print(F("action["));
          Serial.print(x);
          Serial.println(F("].tcPtr->chipName is not used"));
        }
      }
      
      if(action[x].tcPtr != NULL)
      {
        lcd[LCDx]->print(action[x].tcPtr->chipName);
      }else{
        lcd[LCDx]->print(F("NOT USED"));
      }

      lcd[LCDx]->setCursor(tempStrCnt, 2);
      for( y = (tempStrCnt - 1); y < (lcdChars - 4); y++ )
      {
        lcd[LCDx]->print(F(" "));
      }
      lcd[LCDx]->setCursor(16, 2);
      if(action[x].tcPtr->chipStatus == 'N')
      {
        lcd[LCDx]->print(F(" ON "));
      }else{
        lcd[LCDx]->print(F(" OFF"));
      }
      
      lcd[LCDx]->setCursor(0, 3);
      if(action[x].thPtr != NULL)
      {
        tempStrCnt = strlen(action[x].thPtr->chipName);
      }else{
        tempStrCnt= strlen("NOT USED");
      }
      if(setDebug & lcdDebug)
      {
        if(action[x].thPtr != NULL)
        {
          Serial.print(F("tempStrCnt for "));
          Serial.print(action[x].thPtr->chipName);
          Serial.print(F(" is "));
          Serial.println(tempStrCnt);
        }else{
          Serial.print(F("action["));
          Serial.print(x);
          Serial.println(F("].thPtr->chipName is not used"));
        }
      }
      if(action[x].thPtr != NULL)
      {
        lcd[LCDx]->print(action[x].thPtr->chipName);
      }else{
        lcd[LCDx]->print(F("NOT USED"));
      }
      lcd[LCDx]->setCursor(tempStrCnt, 3);
      for( y = (tempStrCnt - 1); y < (lcdChars - 4); y++ )
      {
        lcd[LCDx]->print(F(" "));
      }
      lcd[LCDx]->setCursor(16, 3);
      if(action[x].thPtr->chipStatus == 'N')
      {
        lcd[LCDx]->print(F(" ON "));
      }else{
        lcd[LCDx]->print(F(" OFF"));
      }
      action[x].lcdMillis = millis();
      
      if(setDebug & lcdDebug)
      {
        lcdUpdateStop = millis();
        Serial.print(F("lcdupdate took "));
        Serial.print(lcdUpdateStop - lcdUpdateStart);
        Serial.println(F(" milliseconds"));
      }

    }
  }
}

void EEPROMclear(void)
{
  uint8_t page[pageSize], y;
  char I2CStr[chipNameSize+1];
  uint32_t x;
  lcd[7]->clear();
  lcd[7]->home();
  lcd[7]->print(F(" Clearing I2CEEPROM "));

  for(y = 0; y < pageSize; y++) page[y] = 0xff;
  for(x = 0; x < I2CEEPROMsize; x += pageSize)
  {
    I2CEEPROM_writeAnything(x, page, I2C0x50);
    lcd[7]->setCursor(0, 1);
    sprintf(I2CStr, "0X%04X", (uint16_t) x);
    lcdCenterStr(I2CStr);
    lcd[7]->print(lcdStr);
    lcd[7]->setCursor(0, 2);
    lcdCenterStr("Bytes Cleared");
    lcd[7]->print(lcdStr);
  }
  if(setDebug & eepromDebug)
  {
    Serial.println(F("Exiting readStructures"));
    Serial.println(F("Chip Structure"));
    displayStructure((byte *)(uint32_t) &chip, sizeof(chip), sizeof(chipStruct));
    Serial.println(F("Action Structure"));
    displayStructure((byte *)(uint32_t) &action, sizeof(action), sizeof(chipActionStruct));
    Serial.println(F("PID Structure"));
    displayStructure((byte *)(uint32_t) &ePID, sizeof(ePID), sizeof(chipPIDStruct));
  }
  lcd[7]->setCursor(0, 3);
  lcdCenterStr("Finished");
  lcd[7]->print(lcdStr);
}

void MasterStop(void)
{
  int x;
// turn off all switches
  for(x=0; x<maxChips; x++)
  {
    setSwitch(x, ds2406PIOAoff);
  }
  for(x=0; x<maxActions; x++)
  {
    action[x].actionEnabled = FALSE;
  }
}

void softReset(void)
{
  // 0000101111110100000000000000100
  // Assert [2]SYSRESETREQ
  WRITE_RESTART(0x5FA0004);
}  


void Read_TC_Volts(uint8_t x)
{ 
  if(setDebug & ds2762Debug)
  {
    Serial.println(F("Enter Read_TC_Volts"));
  }
  ds.reset();
  ds.select(chip[x].chipAddr);
  ds.write(0x69); //read voltage
  ds.write(0x0e);
  for (i = 0; i < 2; i++)
  {
    voltage[i] = ds.read();
    if(setDebug & ds2762Debug)
    {
      Serial.print(F("voltage["));
      Serial.print(i);
      Serial.print(F("] = 0x"));
      if(voltage[i] < 0x10){Serial.print(F("0"));}
      Serial.print(voltage[i], HEX);
      Serial.print(F(" "));
    }
  }
  if(setDebug & ds2762Debug)
  {
    Serial.println();
  }
  ds.reset();
  tcVoltage = (voltage[0] << 8) + voltage[1];
  tcVoltage >>= 3; 
  if((voltage[0] & 0x80) == 0x80)
  {
    sign = 1;
    tcVoltage |= 0xF000;
    tcVoltage = ~tcVoltage;
    tcVoltage += 1;
  }else{
    sign = 0;
  }
  tcBuff = tcVoltage * 15;
  tcVoltage *= 5;
  tcVoltage >>= 3;
  tcVoltage += tcBuff;
  
  if(setDebug & ds2762Debug)
  {
    Serial.print(F("tcVoltage = "));
    Serial.print(tcVoltage);
    Serial.println(F(" microvolts"));
    Serial.println(F("Exit Read_TC_Volts"));
  }
} 

/* Reads cold junction (device) temperature 
-- each raw bit = 0.125 degrees C 
-- returns tmpCJ in whole degrees C */ 
void Read_CJ_Temp(uint8_t x)
{ 
  if(setDebug & ds2762Debug)
  {
    Serial.println(F("Enter Read_CJ_Temp"));
  }
  ds.reset();
  ds.select(chip[x].chipAddr);
  ds.write(0x69);
  ds.write(0x18); //read cjTemp
  for (i = 0; i < 2; i++)
  {
    cjTemp[i] = ds.read();
    if(setDebug & ds2762Debug)
    {
      Serial.print(F("cjTemp["));
      Serial.print(i);
      Serial.print(F("] = 0x"));
      if(cjTemp[i] < 0x10){Serial.print(F("0"));}
      Serial.print(cjTemp[i], HEX);
      Serial.print(F(" "));
    }
  }
  if(setDebug & ds2762Debug)
  {
    Serial.println();
  }
  ds.reset();
  cjTemperature = (cjTemp[0] << 8) + cjTemp[1];
  if(cjTemperature>=0x8000)
  { 
    cjTemperature = 0;
//    cjdTemperature = 0.0; // disallow negative 
  }else{
//    cjdTemperature =  (double) ((double) cjTemperature) * .125;
    cjTemperature >>= 8;
  } 
  if(setDebug & ds2762Debug)
  {
    Serial.print(F("cjTemperature = "));
    Serial.print(cjTemperature);
    Serial.print(F(" degrees C, "));
    Serial.print(((cjTemperature * 9) / 5) + 32);
    Serial.println(F(" degrees F")); 
    Serial.println(F("Exit Read_CJ_Temp"));
  }
} 

/* Search currently selected TC table for nearest entry 
-- uses modified binary algorithm to find cjComp 
-- high end of search set before calling (tblHi) 
-- successful search sets tempC */ 
void TC_Lookup(void)
{ 
  if(setDebug & ds2762Debug)
  {
    Serial.println(F("Enter TC_Lookup"));
  }
  tblLo=0; // low entry of table 
  tempC=22; // default to room temp
  testVal=pgm_read_word_near(kTable + tblHi); // check max temp
  if(cjComp>testVal)
  { 
    error=1; // out of range 
  }else{ 
    while(1)
    { 
      eePntr=(tblLo+tblHi)/2; // midpoint of search span 
      testVal=pgm_read_word_near(kTable + eePntr); // read value from midpoint
      if(setDebug & ds2762Debug)
      {
        Serial.print(F("testVal = "));
        Serial.print(testVal);
      }
      if(cjComp == testVal)
      {
        if(setDebug & ds2762Debug)
        {
          Serial.println(F(" - TC_Lookup Temp Match"));
        }
//        tempC = eePntr;
        return; // found it! 
      }else{
        if(cjComp<testVal)
        {
          if(setDebug & ds2762Debug)
          {
             Serial.println(F(" - testVal too BIG"));
          }
         tblHi=eePntr; //search lower half
        }else{
          if(setDebug & ds2762Debug)
          {
             Serial.println(F(" - testVal too small"));
          }
         tblLo=eePntr; // search upper half
        }
      }
      if(setDebug & ds2762Debug)
      {
        Serial.print(F("tblHi = "));
        Serial.print(tblHi);
        Serial.print(F(", tblLo = "));
        Serial.println(tblLo);
      }
      if((tblHi-tblLo)<2)
      { // span at minimum 
        if(setDebug & ds2762Debug)
        {
          Serial.println(F("TC_Lookup Temp Span At Minimum"));
        }
        eePntr=tblLo; 
        return; 
      } 
    } 
  }
}

void lcdCenterStr(char *str)
{
  uint8_t lcdPad;
  for(uint x=0; x<lcdChars; x++) lcdStr[x] = 0x20; // fill lcdStr with spaces
  
  if(strlen(str) > lcdChars+1)
  {
    strcpy(lcdStr, "  ERROR - TOO LONG  ");
    if(setDebug & lcdDebug)
    {
      Serial.println(F("String was too long for the LCD display"));
    }
  }else if(strlen(str) == lcdChars+1){
    strcpy(lcdStr, str);
    if(setDebug & lcdDebug)
    {
      Serial.println(F("String was exactly right for the LCD display"));
    }
  }else{
    
    if(setDebug & lcdDebug)
    {
      Serial.print(F("Input String = "));
      Serial.println(str);
      Serial.print(F("strlen(str) = "));
      Serial.print(strlen(str));
      Serial.println(F(" bytes"));
    }

    lcdPad = (lcdChars - strlen(str)) / 2;
    if(setDebug & lcdDebug)
    {
      Serial.print(F("lcdPad = "));
      Serial.println(lcdPad);
    }

    memcpy(&lcdStr[lcdPad], str, strlen(str));
    if(setDebug & lcdDebug)
    {
      Serial.println(F("String was smaller than the LCD Display"));
      Serial.print(F("lcdStr = \""));
      Serial.print(lcdStr);
      Serial.println(F("\""));
    }
  }
}

