/********************

TeensyNet.ino

Version 0.0.2
Last Modified 11/03/2013
By Jim Mayhugh

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
#include <PID_v1.h>
#include <math.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"
#include <OneWire.h>
#include <errno.h>
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
// #include <EthernetUdp.h>  // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <stdio.h>
#include <stdlib.h>
#include "utility/socket.h"
#include <t3mac.h>
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

/*
  General Setup
*/

// Should restart Teensy 3, will also disconnect USB during restart

// From http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337e/Cihcbadd.html
// Search for "0xE000ED0C"
// Original question http://forum.pjrc.com/threads/24304-_reboot_Teensyduino%28%29-vs-_restart_Teensyduino%28%29?p=35981#post35981

#define RESTART_ADDR       0xE000ED0C
#define READ_RESTART()     (*(volatile uint32_t *)RESTART_ADDR)
#define WRITE_RESTART(val) ((*(volatile uint32_t *)RESTART_ADDR) = (val))


const char* versionStr = "TeensyWiFi CC3000 UDP Server Version 0.0.2, 11/07/2013";

const uint16_t resetDebug      = 0x0001; //    1
const uint16_t pidDebug        = 0x0002; //    2
const uint16_t eepromDebug     = 0x0004; //    4
const uint16_t chipDebug       = 0x0008; //    8
const uint16_t findChipDebug   = 0x0010; //   16
const uint16_t serialDebug     = 0x0020; //   32
const uint16_t udpDebug        = 0x0040; //   64
const uint16_t wifiDebug       = 0x0080; //   128
const uint16_t udpHexBuff      = 0x0100; //   256
const uint16_t chipNameDebug   = 0x0200; //   512
const uint16_t actionDebug     = 0x0400; //  1024


uint16_t setDebug = 0x0050;  

// define serial commands

const uint8_t getMaxChips        = '1';
const uint8_t showChip           = getMaxChips + 1;    // "2"
const uint8_t getChipCount       = showChip + 1;       // "3"
const uint8_t getChipAddress     = getChipCount + 1;   // "4"
const uint8_t getChipStatus      = getChipAddress + 1; // "5"
const uint8_t setSwitchState     = getChipStatus + 1;  // "6"
const uint8_t getAllStatus       = setSwitchState + 1; // "7"
const uint8_t getChipType        = getAllStatus + 1;   // "8"
const uint8_t getAllChips        = getChipType + 1;    // "9" - last in this series

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

// buffers for receiving and sending data
const uint16_t UDP_TX_PACKET_MAX_SIZE = 101;
const uint16_t UDP_RX_PACKET_MAX_SIZE = 101;

char PacketBuffer[UDP_RX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
char ReplyBuffer[1024];  // a string to send back

char PartialBuffer[UDP_TX_PACKET_MAX_SIZE]; // Partial Buffer for oversized messages

char itoaBuf[20];
uint16_t rBuffCnt = 0, tBuffRem = 0, tBuffLoc = 0;
char c;
uint8_t cnt = 0;
uint8_t chipSelected;
uint8_t actionSelected;
uint8_t setChipState;
uint8_t *chipAddrPtr;
bool serialMessageReady = FALSE;
bool actionPtrMatch = FALSE;
bool showCelcius = FALSE;

uint32_t timer, timer2;
const uint32_t updateTime = 250;
const uint32_t ramUpdateTime = 10000;

// OneWire Setup;
const uint8_t wizReset       = 9; // WIZ nic reset
const uint8_t oneWireAddress = 2; // OneWire Bus Address
const uint8_t chipAddrSize   = 8; // 64bit OneWire Address
const uint8_t ds2406MemWr    = 0x55;
const uint8_t ds2406MemRd    = 0xaa;
const uint8_t ds2406AddLow   = 0x07;
const uint8_t ds2406AddHi    = 0x00;
const uint8_t ds2406PIOAoff  = 0x3f;
const uint8_t ds2406PIOAon   = 0x1f;
const uint8_t ds2406End      = 0xff;
const uint8_t ds18b20ID      = 0x28;
const uint8_t ds2406ID       = 0x12;
const uint8_t dsPIO_A        = 0x20;
const uint8_t dsPIO_B        = 0x40;

const uint8_t maxChips       = 12; // Maximum number of Chips
const uint8_t maxActions     = 4; // Maximum number of Actions

OneWire  ds(oneWireAddress);

const uint32_t tempReadDelay = 125;

uint8_t chipAddrArray[chipAddrSize] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

const char *charChipAddrArray = "0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00";
const char *unassignedStr = "_____UNASSIGNED_____";

const uint8_t chipNameSize = 20;

typedef struct
{
  uint8_t   chipAddr[chipAddrSize];
  int16_t   chipStatus;
  uint32_t  tempTimer;
  char      chipName[chipNameSize+1];
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
}chipActionStruct;

const chipActionStruct actionClear = { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0 };

chipActionStruct action[maxActions] =
{
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0 }
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

PID *pidArrayPtr[] = {&PID0,&PID1,&PID2,&PID3};

// End PID Stuff

//EEPROM Stuff
const int   EEPROMsize       = 2048;   // Cortex M4
const int   EEPROMidAddr     = 0x10;   // ID address to verify a previous EEPROM write
const int   EEPROMccAddr     = 0x20;   // number of chips found during findchips()
const int   EEPROMchipAddr   = 0x40;  // start address of structures
const byte  EEPROMidVal      = 0x55;   // Shows that an EEPROM update has occurred 
bool        eepromReady      = FALSE;
int         eepromSpace, eeResult, EEPROMactionAddr, EEPROMpidAddr;

// Wireless UDP Stuff
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
// Define CC3000 chip pins
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  4
#define ADAFRUIT_CC3000_CS    10
 
const unsigned long
  dhcpTimeout     = 60L * 1000L, // Max time to wait for address from DHCP
  connectTimeout  = 15L * 1000L, // Max time to wait for server connection
  responseTimeout = 15L * 100000L; // Max time to wait for data from server
uint32_t t;

// WiFi network (change with your settings !)
#define WLAN_SSID             "GMJLinksys"        // cannot be longer than 32 characters!
#define WLAN_PASS             "ckr7518t"
#define WLAN_SECURITY         WLAN_SEC_WPA2 // This can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
//#define  SOL_SOCKET           0xffff
//#define  SOCKOPT_RECV_TIMEOUT 1 
// Create CC3000 instances
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIV2);                                
Adafruit_CC3000_Server server;
                                         
// Local server IP, port, and repository (change with your settings !)
// uint32_t ip = cc3000.IP2U32(192,168,1,11);
unsigned int localPort = 2652;      // local port to listen on


unsigned long startTime;
uint16_t rbCount = 0;

// LCD Stuff

// The shield uses the I2C SCL and SDA pins. 
// You can connect other I2C sensors to the I2C bus and share
// the I2C bus.

Adafruit_RGBLCDShield lcd0 = Adafruit_RGBLCDShield(0);
Adafruit_RGBLCDShield lcd1 = Adafruit_RGBLCDShield(1);
Adafruit_RGBLCDShield lcd2 = Adafruit_RGBLCDShield(2);
Adafruit_RGBLCDShield lcd3 = Adafruit_RGBLCDShield(3);
Adafruit_RGBLCDShield lcd4 = Adafruit_RGBLCDShield(4);
Adafruit_RGBLCDShield lcd5 = Adafruit_RGBLCDShield(5);
Adafruit_RGBLCDShield lcd6 = Adafruit_RGBLCDShield(6);
Adafruit_RGBLCDShield lcd7 = Adafruit_RGBLCDShield(7);

// These #defines make it easy to set the backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

// End LCD Stuff

void setup()
{
  Serial.begin(baudRate);
  
  if(setDebug > 0x0)
  {  
    delay(3000);
  }
  
  Serial.print(F("Serial Debug running at "));
  Serial.print(baudRate);
  Serial.println(F(" baud"));


  eeResult = EEPROM.read(EEPROMidAddr);
  
  if(setDebug & eepromDebug)
  { 
    Serial.print(F("eeResult = 0x"));
    Serial.println(eeResult, HEX);
  }
  
  if(eeResult != 0x55)
  {
    if(setDebug & eepromDebug)
    { 
       Serial.println(F("No EEPROM Data"));
    }
  
    eepromReady = FALSE;
    findChips();
    saveStructures();
  }else{

    if(setDebug & eepromDebug)
    { 
      Serial.println(F("Getting EEPROM Data"));
    }

    chipCnt = EEPROM.read(EEPROMccAddr);
    readStructures();
    if(setDebug & eepromDebug)
    { 
      Serial.println(F("EEPROM Data Read Completed"));
    }
  
    eepromReady = TRUE;
    
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

  delay(1000);

  if(setDebug & udpDebug)
  {
    Serial.println(F("Initializing CC3000 nic"));
  }
  // Initialise the CC3000 module
  if (!cc3000.begin())
  {
    while(1);
  } 

// Per http://e2e.ti.com/support/low_power_rf/f/851/t/292664.aspx
// aucInactivity needs to be set to 0 (never timeout) or the socket will close after
// 60 seconds of no activity
  unsigned long aucDHCP       = 0xffffffff; //no timeout
  unsigned long aucARP        = 3600;
  unsigned long aucKeepalive  = 30;
  unsigned long aucInactivity = 0; // no timeout
  if(aucInactivity == 0)
  {
    Serial.println(F("Setting netapp to not timeout..."));
  }else{
    Serial.print(F("Setting netapp to timeout in "));
    Serial.print(aucInactivity);
    Serial.println(F(" Seconds"));
  }
  long iRet = netapp_timeout_values(&aucDHCP, &aucARP, &aucKeepalive, &aucInactivity);
  if (iRet!=0)
  {
    Serial.print(F("Could not set netapp option, iRet = "));
    Serial.println(iRet);
    Serial.println(F(", aborting..."));
    while(1);
  }else{
    Serial.print(F("set netapp option, iRet = "));
    Serial.println(iRet);
  }

  displayMACAddress();
  
  // Connect to  WiFi network
  cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY);
  Serial.println("Connected to WiFi network!");


  // Check DHCP
  Serial.print(F("Requesting address from DHCP server..."));
  for(t=millis(); !cc3000.checkDHCP() && ((millis() - t) < dhcpTimeout); delay(1000));
  if(cc3000.checkDHCP()) {
    Serial.println(F("OK"));
    displayConnectionDetails();
  } else {
    Serial.println(F("failed"));
    return;
  }

  timer = millis();
  timer2 = millis();
  
  for(int q = 0; q < UDP_TX_PACKET_MAX_SIZE; q++) // clear the buffer udp buffers
  {
    ReplyBuffer[q] = 0x00;
    PacketBuffer[q] = 0x00;
  }
  unsigned long startTime;

  Serial.println(F("Attempting connection..."));
  
  startTime = millis();
    
    do
    {
      server = cc3000.connectAndBindUDP(0x0, localPort);
    } while((!server.connected()) &&
            ((millis() - startTime) < connectTimeout));

    if(server.connected())
    {
      Serial.println(F("connected!"));
    }else{
      Serial.println(F("NOT connected!"));
      while(1);
    }
    
    lcd7.begin(20, 4);
    lcd7.clear();
    lcd7.home();
    lcd7.print(F("   Hello World!    "));
    lcd7.setCursor(0, 2);
    lcd7.print(F(" My IP address is:  "));
    lcd7.setCursor(0, 3);
    lcd7.print(F("   "));
//    lcd7.print(Ethernet.localIP());
    lcd7.print(F("   "));

  pidSetup();
}

void loop()
{
  uint16_t rpBufferCnt = 0;

  if(!server.connected())
  {
    if(setDebug & udpDebug)
    {
      Serial.println(F("server not connected"));
    }
  }
  
  while(server.available())
  {
    char c = server.read();
    
    if(c == '='){continue;}
    
    if(c != 0x00)
    {
      PacketBuffer[rpBufferCnt] = c;
      if(PacketBuffer[rpBufferCnt] == '\n')
      {
        break;
      }else{
        rpBufferCnt++;
      }
    }
  }
  
  if(PacketBuffer[0] != 0x00)
  {
    if(setDebug & udpDebug)
    {
      Serial.println();
      Serial.print(F("PacketBuffer = "));
      Serial.println(PacketBuffer);
      Serial.println(F("starting udpProcess()"));
    }
    udpProcess();
  }


  if(timer > (millis() + 5000)) // in case of rollover
  {
    timer = millis();
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
  eepromSpace = 0;

  if(setDebug & eepromDebug)
  {
    Serial.println(F("Entering readStructures"));
    Serial.print(F("EEPROMchipAddr = 0x"));
    Serial.println(EEPROMchipAddr, HEX);
  }

  eeResult = EEPROM_readAnything(EEPROMchipAddr, chip);

  eepromSpace += eeResult;

  if(setDebug & eepromDebug)
  {
    Serial.print(F("Read "));
    Serial.print(eeResult);
    Serial.print(F(" bytes from address Ox"));
    Serial.println(EEPROMchipAddr, HEX);
  }

  EEPROMactionAddr = (eeResult + EEPROMchipAddr + 0x10) & 0xFFFF0;

  if(setDebug & eepromDebug)
  {
    Serial.print(F("EEPROMactionAddr = 0x"));
    Serial.println(EEPROMactionAddr, HEX);
  }

  eeResult = EEPROM_readAnything(EEPROMactionAddr, action);

  eepromSpace += eeResult;

  if(setDebug & eepromDebug)
  {
    Serial.print(F("Read "));
    Serial.print(eeResult);
    Serial.print(F(" bytes from address Ox"));
    Serial.println(EEPROMactionAddr, HEX);
  }

  EEPROMpidAddr =  (EEPROMactionAddr + eeResult + 0x10) & 0xFFFF0;

  if(setDebug & eepromDebug)
  {
    Serial.print(F("EEPROMpidAddr = 0x"));
    Serial.println(EEPROMpidAddr, HEX);
  }

  eeResult = EEPROM_readAnything(EEPROMpidAddr, ePID);

  eepromSpace += eeResult;

  if(setDebug & eepromDebug)
  {
    Serial.print(F("Read "));
    Serial.print(eeResult);
    Serial.print(F(" bytes from address Ox"));
    Serial.println(EEPROMpidAddr, HEX);
    Serial.print(F("readStructures() EEPROM Data Read of "));
    Serial.print(eepromSpace);
    Serial.println(F(" bytes Completed"));
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
    Serial.print(F("EEPROMchipAddr = 0x"));
    Serial.println(EEPROMchipAddr, HEX);
  }
  eepromSpace = 0;
  EEPROM.write(EEPROMccAddr, chipCnt);
  EEPROM.write(EEPROMidAddr, EEPROMidVal);
  eeResult = EEPROM_writeAnything(EEPROMchipAddr, chip);
  if(setDebug & eepromDebug)
  {
    Serial.print(F("Wrote "));
    Serial.print(eeResult);
    Serial.print(F(" bytes to address Ox"));
    Serial.println(EEPROMchipAddr, HEX);
  }
  eepromSpace += eeResult;
  EEPROMactionAddr = (eeResult + EEPROMchipAddr + 0x10) & 0xFFFF0;
  if(setDebug & eepromDebug)
  {
    Serial.print(F("EEPROMactionAddr = 0x"));
    Serial.println(EEPROMactionAddr, HEX);
  }
  eeResult = EEPROM_writeAnything(EEPROMactionAddr, action);
  eepromSpace += eeResult;
  if(setDebug & eepromDebug)
  {
    Serial.print(F("Wrote "));
    Serial.print(eeResult);
    Serial.print(F(" bytes to address Ox"));
    Serial.println(EEPROMactionAddr, HEX);
  }
  EEPROMpidAddr =  (EEPROMactionAddr + eeResult + 0x10) & 0xFFFF0;
  if(setDebug & eepromDebug)
  {
    Serial.print(F("EEPROMpidAddr = 0x"));
    Serial.println(EEPROMpidAddr, HEX);
  }
  eeResult = EEPROM_writeAnything(EEPROMpidAddr, ePID);
  eepromSpace += eeResult;
  if(setDebug & eepromDebug)
  {
    Serial.print(F("Wrote "));
    Serial.print(eeResult);
    Serial.print(F(" bytes to address Ox"));
    Serial.println(EEPROMpidAddr, HEX);
    Serial.print(F("saveStructures() EEPROM Data Write of "));
    Serial.print(eepromSpace);
    Serial.println(F(" bytes Completed - Displaying chip Structures"));
    displayStructure((byte *)(uint32_t) &chip, sizeof(chip), sizeof(chipStruct));
    Serial.println(F("Action Structure"));
    displayStructure((byte *)(uint32_t) &action, sizeof(action), sizeof(chipActionStruct));
    Serial.println(F("PID Structure"));
    displayStructure((byte *)(uint32_t) &ePID, sizeof(ePID), sizeof(chipPIDStruct));
    Serial.println(F("Exiting saveStructures"));
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

void findChips()
{
 int cntx = 0, cnty, cmpCnt, cmpArrayCnt, dupArray = 0;

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
      dupArray = 0;
      continue;
    }
    
    if(ds.crc8(chip[cntx].chipAddr, chipAddrSize-1) != chip[cntx].chipAddr[chipAddrSize-1]) continue;

    if(setDebug & findChipDebug)
    {
      Serial.print(F("Chip "));
      Serial.print(cntx);
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
  }

  if(setDebug & findChipDebug)
  {
    Serial.print(cntx);
    Serial.print(F(" Sensor"));
    if(cntx == 1)
    {
      Serial.println(F(" Detected"));
    }else{
      Serial.println(F("s Detected"));
    }
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
}

void sendUDPpacket(void)
{
  uint16_t v = 0, q = 0;
  uint8_t tPartialCnt = 0;
  
  if( (setDebug & udpDebug) || (setDebug & wifiDebug) )
  {
    Serial.print(F("rBuffCnt = "));
    Serial.println(rBuffCnt);
    Serial.println(F("ReplyBuffer:"));
//    Serial.println(ReplyBuffer);
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
    if(server.connected())
    {
      // Assemble and issue request packet
      if(rBuffCnt > UDP_TX_PACKET_MAX_SIZE)
      {
        if( (setDebug & udpDebug) || (setDebug & wifiDebug) )
        {
          Serial.println(F("ReplyBuffer too large"));
        }
        tBuffRem = 0;
        tBuffLoc = 0;
        if( (setDebug & udpDebug) || (setDebug & wifiDebug) )
        {
          Serial.println(F("Sending Large Packet identifier"));
        }
        tPartialCnt = sprintf(PartialBuffer, "%c", '=');
        server.write(PartialBuffer, tPartialCnt);
        
        while(!server.available());

        if( (setDebug & udpDebug) || (setDebug & wifiDebug) )
        {
          Serial.println(F("Sending First Packet"));
        }
        tPartialCnt = server.write(ReplyBuffer, UDP_TX_PACKET_MAX_SIZE);
        if( (setDebug & udpDebug) || (setDebug & wifiDebug) )
        {
          Serial.write((const unsigned char*) ReplyBuffer, UDP_TX_PACKET_MAX_SIZE);
          Serial.println();
          Serial.print(tPartialCnt);
          Serial.println(F(" characters sent"));
        }
        
        tBuffRem = rBuffCnt - tPartialCnt;
        if( (setDebug & udpDebug) || (setDebug & wifiDebug) )
        {
          Serial.print(tBuffRem);
          Serial.println(F(" characters left to send"));
        }
        
        while(tBuffRem >= UDP_TX_PACKET_MAX_SIZE)
        {
          if(server.read() == '=')
          {
            tBuffLoc += UDP_TX_PACKET_MAX_SIZE;
            if( (setDebug & udpDebug) || (setDebug & wifiDebug) )
            {
              Serial.print(F("tBuffLoc = "));
              Serial.println(tBuffLoc);
              Serial.println(F("Sending Next Packet"));
            }
            tPartialCnt = server.write((char *) &ReplyBuffer[tBuffLoc], UDP_TX_PACKET_MAX_SIZE);
            if( (setDebug & udpDebug) || (setDebug & wifiDebug) )
            {
              Serial.write((const unsigned char *) &ReplyBuffer[tBuffLoc], UDP_TX_PACKET_MAX_SIZE);
              Serial.println();
              Serial.print(tPartialCnt);
              Serial.println(F(" more characters sent"));
            }
            tBuffRem -= UDP_TX_PACKET_MAX_SIZE;
          }
        }
        
        if(tBuffRem > 0)
        {
          if(server.read() == '=')
          {
            if( (setDebug & udpDebug) || (setDebug & wifiDebug) )
            {
              Serial.print(F("tBuffRem = "));
              Serial.println(tBuffRem);
              Serial.println(F("Sending Remaining Packet"));
            }
            tBuffLoc += UDP_TX_PACKET_MAX_SIZE;
            if( (setDebug & udpDebug) || (setDebug & wifiDebug) )
            {
              Serial.print(F("tBuffLoc = "));
              Serial.println(tBuffLoc);
              Serial.println(F("Sending Remaining Packet"));
            }
            tPartialCnt = server.write((char *) &ReplyBuffer[tBuffLoc], tBuffRem);
            if( (setDebug & udpDebug) || (setDebug & wifiDebug) )
            {
              Serial.write((const unsigned char *) &ReplyBuffer[tBuffLoc], tBuffRem);
              Serial.println();
              Serial.print(tPartialCnt);
              Serial.println(F(" characters sent"));
            }
          }
        }
        server.read();
        server.write('+');
        if( (setDebug & udpDebug) || (setDebug & wifiDebug) )
        {
          Serial.println(F("Done"));
        }
      }else{
        if(rBuffCnt > 0)
        {
          server.write(ReplyBuffer, rBuffCnt);
        }else{
          server.write("OK\n", 3);
        }
      }
    }else{
        if( (setDebug & udpDebug) || (setDebug & wifiDebug) )
        {
          Serial.println(F("Can't Send Reply, server not connected"));
        }
    }
    
  if( (setDebug & udpDebug) || (setDebug & wifiDebug) )
  {
    Serial.println(F("Clearing UDP buffers"));
  }
  
  for(q = 0; q < UDP_TX_PACKET_MAX_SIZE; q++) // clear the udp buffers
  {
    ReplyBuffer[q] = 0x00;
    PacketBuffer[q] = 0x00;
  }  
}

void udpProcess()
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
 
    case getAllChips: // "9"
    {
      for(x = 0; x < maxChips; x++)
      {
        showChipInfo(x);
        if(x < maxChips -1)
        {
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",";");
        }
      }
      if(setDebug & udpDebug)
      {
        Serial.print(F("rBuffCnt = "));
        Serial.println(rBuffCnt);
        Serial.println(ReplyBuffer);
      }
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
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","\n");
      }
      sendUDPpacket();
    }
    break;
    
    case updateActionArray: // "B"
    {
      
      result = strtok( PacketBuffer, delim );

      while(1)
      {
        result = strtok( NULL, delim );
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
                Serial.print(F("result = "));
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
          
          case 4:
          {
            if(setDebug & actionDebug)
            {
              Serial.print(F("Case 4 addrResult = "));
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
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","EEPromSaved");
      sendUDPpacket();
      break;
    }
    
    case getEEPROMstatus: // "G"
    {
      if(eepromReady == FALSE)
      {
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","FALSE");
      }else
      {
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
      setDebug = 0x0000;
      for(uint8_t x = 1; x < 5; x++)
      {
        uint16_t y = 0;
        switch(PacketBuffer[x])
        {
          case 'A':
          case 'B':
          case 'C':
          case 'D':
          case 'E':
          case 'F':
          {
            y = (uint16_t)(PacketBuffer[x] - 0x37);
            break;
          }
          
          case 'a':
          case 'b':
          case 'c':
          case 'd':
          case 'e':
          case 'f':
          {
            y = (PacketBuffer[x] - 0x57);
            break;
          }
          
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
          {
            y = (PacketBuffer[x] - 0x30);
            break;
          }
          
          default:
          {
            y = 0;
            break;
          }
        }
        setDebug |= y;
        if(x < 4){setDebug <<= 4;}  
      }
//      setDebug = atoi((char *) &PacketBuffer[1]);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","0x");
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%04x",setDebug);
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
      if( x >= maxChips )
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
      
      strcpy(chip[chipAddrCnt].chipName, chipNameStr);

      if(setDebug & chipNameDebug)
      {
        Serial.print(F("chip["));
        Serial.print(chipAddrCnt);
        Serial.print(F("].chipName = "));
        Serial.println(chip[chipAddrCnt].chipName);
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
      eeResult = EEPROM.read(EEPROMidAddr);
  
      if(setDebug & resetDebug)
      { 
        Serial.print(F("eeResult = 0x"));
        Serial.println(eeResult, HEX);
      }
  
      if(eeResult != 0x55)
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
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",versionStr);
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
  for( int i = 0; i < chipAddrSize; i++)
  {
    rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s", "0x");    
    rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%02X",(uint8_t) array[i]);
    if(i < 7)
    {
      rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s",",");
    }
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
  
  switch(chip[x].chipAddr[0])
  {
    case ds18b20ID:
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
        ds.write(0x44,1);         // start conversion, with parasite power on at the end
        chip[x].tempTimer = millis();
      }
/*    
      delay(125);     // for 9 bit accuracy
      // we might do a ds.depower() here, but the reset will take care of it.
*/    
      if((chip[x].tempTimer != 0) && (millis() >= chip[x].tempTimer + tempReadDelay))
      {
        ds.reset();
        ds.select(chip[x].chipAddr);    
        ds.write(0xBE);         // Read Scratchpad
  
        for (int i = 0; i < 9; i++) 
        {
          chipBuffer[i] = ds.read();
        }
        
        if(ds.crc8(chipBuffer, 8) != chipBuffer[8]) break; // CRC invalid, try later
  
      // convert the data to actual temperature
        unsigned int raw = (chipBuffer[1] << 8) | chipBuffer[0];
        if( showCelcius == TRUE)
        {
          chip[x].chipStatus = (int) ((float)raw / 16.0);
        }else{
          chip[x].chipStatus = (int) ((((float)raw / 16.0) * 1.8) + 31.0);
        }
        chip[x].tempTimer = 0;
      }
    }
    break;
    
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
/*
          Serial.print(F("chipBuffer = "));
          for(int i = 0; i < 13; i++)
          {
            if(chipBuffer[i] >= 0 && chipBuffer[i] <= 15)
            {
              Serial.print(F("0X0"));
            }else{
              Serial.print(F("0X"));
            }
            Serial.print(chipBuffer[i], HEX);
            if(i < 13)
            {
              Serial.print(F(", "));
            }else{
              Serial.println();
            }
          }
*/
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
    }
    break;
    
    default:
    {
      chip[x].chipStatus = noChipPresent;
    }
  break; 
  }
}

void updateActions(uint8_t x)
{

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
  }
}

void EEPROMclear(void)
{
  for(int e = 0; e < EEPROMsize; e++)
  {
    EEPROM.write(e, 0x0);
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

void softReset()
{
  // 0000101111110100000000000000100
  // Assert [2]SYSRESETREQ
  WRITE_RESTART(0x5FA0004);
}  

void displayMACAddress(void)
{
  uint8_t macAddress[6];
  
  if(!cc3000.getMacAddress(macAddress))
  {
    Serial.println(F("Unable to retrieve MAC Address!\r\n"));
  }
  else
  {
    Serial.print(F("MAC Address : "));
    cc3000.printHex((byte*)&macAddress, 6);
  }
}

bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}


