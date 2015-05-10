/********************

TeensyNet.ino

Version 0.0.56
Last Modified 05/10/2015
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

/*********************
This code requires a change in the appropriate EthernetUDP.h library file to work properly.
This is due primarily to the limited amount of RAM that's available in the Arduino series versus the Teensy.

------------------------------------------
find all instances or EthernetUDP.h and replace:

#define UDP_TX_PACKET_MAX_SIZE 24

with:

#if defined(__MK20DX256__) || defined(__MK20DX128__)
#define UDP_TX_PACKET_MAX_SIZE 2048
#else
#define UDP_TX_PACKET_MAX_SIZE 24
#endif

*********************/

#define USEVGAONLY 1 // if set to 1, use VGA Values ONLY, otherwise use VGA or RGB
#define USESERIAL2 0 // if set to 1, use Serial2 with FTDI for debugging otherwise use the Arduino IDE Serial Monitor

#include <PID_v1.h>
#include <math.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <errno.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>  // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <t3mac.h>
#include <i2c_t3.h>
#include <Teensy_MCP23017.h>
#include <Teensy_RGBLCDShield.h>
#include <TeensyNetGLCD.h> // TeensyGLCD structures and commands
#include <I2CEEPROMAnything.h>
#include <EthernetBonjour.h>
#include <TeensyNetFuncProt.h> // function prototype list
#include <TeensyDebug.h>
#include <TeensyUDP.h>
#include <Teensy1Wire.h>
#include <TeensyAction.h>
#include <TeensyPID.h>
#include <TeensyI2CEEPROM.h>
#include <TeensyLCDShield.h>
#include <TeensyBonjour.h>
#include <TeensyEthernet.h>
#include <TeensyDS2762.h>
#include <TeensyNetLCD.h>
#include <TeensyTestPoints.h>
#include <TeensyVersionInfo.h>

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



const uint8_t hwMasterStopPin  = 22;
const uint8_t chipResetPin     = 23;
const uint8_t w5200ResetPin    = 9;

OneWire  ds(oneWireAddress);


void setup()
{
  int x;

  for(x = LED1; x <= LED5; x++)
  {
    pinMode(x,OUTPUT);       // test LEDs on TeensyNet > V12.0
    setLED(x, ledOFF);
  }

  Wire.begin();  // enable the 1st I2C port
#if __MK20DX256__ 
  Wire1.begin(); // enable the 2nd I2C port
#endif

// reassign pins 26 and 31 to use the ALT3 configuration
// which makes them  Serial port 2 Rx(26) and Tx(31)
  CORE_PIN26_CONFIG = PORT_PCR_MUX(3);
  CORE_PIN31_CONFIG = PORT_PCR_MUX(3);
  

// the serial debug port can be chosen with the USESERIAL2 #define
  Serial2.begin(baudRate); // start the optional FTDI serial port for debug
  Serial.begin(baudRate); // start the default Arduino IDE Serial Monitor debug port

#if USESERIAL2 == 1
  debugPort = 1;
#else
  debugPort = 0;
#endif

  
  pinMode(hwMasterStopPin, INPUT_PULLUP); // set to monitor Master Stop switch
  pinMode(chipResetPin, INPUT_PULLUP); // set to monitor Reset switch
    
  delay(1000);
  
  myDebug[debugPort]->print(F("Serial Debug starting at "));
  myDebug[debugPort]->print(baudRate);
  myDebug[debugPort]->println(F(" baud"));
  myDebug[debugPort]->print(F("UDP_TX_PACKET_MAX_SIZE = "));
  myDebug[debugPort]->println(UDP_TX_PACKET_MAX_SIZE);

  for(x = 0; x < numLCDs; x++)
  {
    lcd[x]->begin(lcdChars, lcdRows);
    lcd[x]->clear();
    lcd[x]->home();
    lcd[x]->print(F("Serial Debug = "));
    lcd[x]->print(baudRate);
    lcd[x]->setCursor(0, 2);
    sprintf(lcdStrBuf, "%s%s", teensyType, versionStrNumber);
    lcdCenterStr((char *) lcdStrBuf, lcdChars);
    lcd[x]->print(lcdStr);
  }

  delay(3000);

#if !defined(STATIC_IP)
  I2CEEPROM_readAnything(I2CEEPROMidAddr, i2cEeResult, I2C0x50);

  if(setDebug & eepromDebug)
  { 
    myDebug[debugPort]->print(F("i2cEeResult = 0x"));
    myDebug[debugPort]->println(i2cEeResult, HEX);
  }
  
  I2CEEPROM_readAnything(I2CEEPROMipAddr, i2cIPResult, I2C0x50);
  
  if(setDebug & eepromDebug)
  { 
    myDebug[debugPort]->print(F("i2cipResult = "));
    myDebug[debugPort]->print(i2cIPResult[0]);
    myDebug[debugPort]->print(F("."));
    myDebug[debugPort]->print(i2cIPResult[1]);
    myDebug[debugPort]->print(F("."));
    myDebug[debugPort]->print(i2cIPResult[2]);
    myDebug[debugPort]->print(F("."));
    myDebug[debugPort]->println(i2cIPResult[3]);
  }
#endif // STATIC_IP

  I2CEEPROM_readAnything(I2CEEPROMbjAddr, bonjourNameBuf, I2C0x50);
  
  if(bonjourNameBuf[0] >= 0x20 && bonjourNameBuf[0] <= 0x7a)
  { 
    if(setDebug & eepromDebug)
    { 
      myDebug[debugPort]->print(F("bonjourNameBuf = "));
      myDebug[debugPort]->println(bonjourNameBuf);
    }
  }else{
    for(int x = 0; x < chipNameSize; x++)
    {
      bonjourNameBuf[x] = 0x0;
    }
    if(setDebug & eepromDebug)
    { 
      myDebug[debugPort]->println(F("bonjourNameBuf Cleared"));
    }
  }

  lcd[7]->clear();
  lcd[7]->home();
  
  if(i2cEeResult != 0x55)
  {
    if(setDebug & eepromDebug)
    { 
       myDebug[debugPort]->println(F("No I2CEEPROM Data"));
    }

    lcd[7]->print(F(" No I2CEEPROM Data  "));
  
    i2cEepromReady = FALSE;
    findChips();
    saveStructures();
  }else{
    if(setDebug & eepromDebug)
    { 
      myDebug[debugPort]->println(F("Getting EEPROM Data"));
    }

    lcd[7]->print(F("Valid I2CEEPROM Data"));
    
    I2CEEPROM_readAnything(I2CEEPROMchipCntAddr, chipCnt, I2C0x50);
    
    if(setDebug & eepromDebug)
    { 
      myDebug[debugPort]->print(F("chipCnt at I2CEEPROMccAddr =  "));
      myDebug[debugPort]->println(chipCnt);
    }

    readStructures();
    if(setDebug & eepromDebug)
    { 
      myDebug[debugPort]->println(F("I2CEEPROM Data Read Completed"));
    }

    lcd[7]->setCursor(0, 3);
    lcd[7]->print(F("I2CEEPROM Read Done "));
  
    i2cEepromReady = TRUE;
    
  }
  
  if(setDebug & eepromDebug)
  { 
    myDebug[debugPort]->print( (sizeof(chipStruct) / sizeof(byte) ) * maxChips);
    myDebug[debugPort]->println(F(" bytes in chip structure array"));
    myDebug[debugPort]->print( (sizeof(chipActionStruct) / sizeof(byte) ) *maxActions);
    myDebug[debugPort]->println(F(" bytes in action structure array"));
    myDebug[debugPort]->print( (sizeof(chipPIDStruct) / sizeof(byte) ) *maxPIDs);
    myDebug[debugPort]->println(F(" bytes in pid structure Array"));
    myDebug[debugPort]->print( (sizeof(glcd1wStruct) / sizeof(byte) ) * maxGLCDs);
    myDebug[debugPort]->println(F(" bytes in glcd structure array"));
    myDebug[debugPort]->print( (sizeof(t3LCDStruct) / sizeof(byte) ) * maxLCDs);
    myDebug[debugPort]->println(F(" bytes in lcd structure array"));
  }

  delay(3000);
  
  if(setDebug & udpDebug)
  {
    myDebug[debugPort]->println(F("Configuring IP"));
  }

  lcd[7]->clear();
  lcd[7]->home();
  lcd[7]->print(F("   Configuring IP   "));

  // start the Ethernet and UDP:
  read_mac();
  
  if(setDebug & udpDebug)
  {
    myDebug[debugPort]->print(F("MAC Address = "));
    print_mac();
    myDebug[debugPort]->println();
  }
  
  delay(1000);
  // reset the wiz820io
  pinMode(w5200ResetPin, OUTPUT);
  digitalWrite(w5200ResetPin, LOW);
  delay(1);
  digitalWrite(w5200ResetPin, HIGH);
  delay(1000);
  
#ifdef STATIC_IP  
  Ethernet.begin((uint8_t *) &mac, ip); // use this for static IP
  Udp.begin(localPort);
#else
  switch(i2cIPResult[0])
  {
    case 0x00:
    case 0xff:
    {
      if(Ethernet.begin((uint8_t *) &mac) == 0) // use this for dhcp
      {
        lcd[7]->setCursor(0, 1);
        lcd[7]->print(F("  IP Config Failed  "));
        lcd[7]->setCursor(0, 2);
        lcd[7]->print(F("Resetting TeensyNet "));
        myDebug[debugPort]->println(F("Ethernet,begin() failed - Resetting TeensyNet"));
        delay(1000);
        softReset();
      }else{
        myDebug[debugPort]->println(F("Ethernet,begin() dhcp success"));
        Udp.begin(localPort);

        myDebug[debugPort]->print(F("My IP address: "));
        myDebug[debugPort]->println(Ethernet.localIP());
        for(uint8_t l = 0; l < 4; l++)
        {
          i2cIPResult[l] = Ethernet.localIP()[l];
        }
        I2CEEPROM_writeAnything(I2CEEPROMipAddr, i2cIPResult, I2C0x50);
        
        if(!(bonjourNameBuf[0] >= 0x20 && bonjourNameBuf[0] <= 0x7a))
        {
          sprintf(bonjourNameBuf, "TeensyNet%d", Ethernet.localIP()[3]);
        }
        
        break;
      }
    }
    
    default:
    {
      IPAddress i2cIPAddr(i2cIPResult[0],i2cIPResult[1],i2cIPResult[2],i2cIPResult[3]);
      Ethernet.begin((uint8_t *) &mac, i2cIPAddr); // use this for IP address from I2CEEPROM
      Udp.begin(localPort);
      myDebug[debugPort]->println(F("Ethernet,begin() from I2CEEPROM success"));
      myDebug[debugPort]->print(F("My IP address: "));
      myDebug[debugPort]->println(Ethernet.localIP());
      break;
    }
  }
#endif
// start Bonjour service
  setLED(LED5, ledON);
  if(EthernetBonjour.begin(bonjourNameBuf))
  {
    myDebug[debugPort]->println(F("Bonjour Service started"));
    sprintf(bonjourBuf, "%s._discover", bonjourNameBuf);
    EthernetBonjour.addServiceRecord(bonjourBuf, localPort, MDNSServiceUDP);
    I2CEEPROM_writeAnything(I2CEEPROMbjAddr, bonjourNameBuf, I2C0x50);
  }else{
    myDebug[debugPort]->println(F("Bonjour Service failed"));
  }
  EthernetBonjour.run();
  setLED(LED5, ledOFF);

// send startup data to I2C LCDs if available
  for(x = 0; x < numLCDs; x++)
  {
    lcd[x]->clear();
    lcd[x]->home();
    lcdCenterStr((char *) bonjourNameBuf, lcdChars);
    lcd[x]->print(lcdStr);
    lcd[x]->setCursor(0, 1);
    sprintf(lcdStrBuf, "%s%s", teensyType, versionStrNumber);
    lcdCenterStr((char *) lcdStrBuf, lcdChars);
    lcd[x]->print(lcdStr);
    lcd[x]->setCursor(0, 2);
    lcd[x]->print(F("   "));
    lcd[x]->print(Ethernet.localIP());
    lcd[x]->print(F("   "));
  }

// send startup data to 1-Wire LCDs if available  
  for(x = 0; x < maxLCDs; x++)
  {
    if(lcd1w[x].Addr[0] == dsLCD)
    {
      lcdCenterStr((char *) bonjourNameBuf, lcdChars);
      sendLCDcommand(x, clr1wLCD, 0, 0, nullStr, 1); // clear the 1wLCD
      sendLCDcommand(x, prt1wLCD, 0, 0, lcdStr, 1); // send the bonjour name
      sprintf(lcdStrBuf, "%s%s", teensyType, versionStrNumber);
      lcdCenterStr((char *) lcdStrBuf, lcdChars);
      sendLCDcommand(x, prt1wLCD, 1, 0, lcdStr, 1); // send the Teensy type and Version Number
      sprintf(lcdStrBuf, "%d.%d.%d.%d", i2cIPResult[0],i2cIPResult[1],i2cIPResult[2],i2cIPResult[3]);
      lcdCenterStr((char *) lcdStrBuf, lcdChars);
      sendLCDcommand(x, prt1wLCD, 2, 0, lcdStr, 1); // send the Local IP Address
    }
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
  
  for(uint8_t q = 0; q < maxGLCDs; q++)
  {
    resetGLCDdisplay(q);
  }
}

void loop()
{
  packetSize = Udp.parsePacket();
  if(packetSize)
  {
    if(setDebug & udpDebug)
    {
      myDebug[debugPort]->print(F("\nLoop:\nReceived packet of size: "));
      myDebug[debugPort]->println(packetSize);
    }
    for(int pbSize = packetSize; pbSize < UDP_TX_PACKET_MAX_SIZE; pbSize++)
    {
      PacketBuffer[pbSize] = 0; // clear the remainder of the buffer
    }
    if(setDebug & udpDebug)
    {
      myDebug[debugPort]->print("From ");
      IPAddress remote = Udp.remoteIP();
      for (int i =0; i < 4; i++)
      {
        myDebug[debugPort]->print(remote[i], DEC);
        if (i < 3)
        {
          myDebug[debugPort]->print(".");
        }
      }
      myDebug[debugPort]->print(", port ");
      myDebug[debugPort]->println(Udp.remotePort());
    }
    // read the packet into packetBufffer
    Udp.read(PacketBuffer,UDP_TX_PACKET_MAX_SIZE);
    if(setDebug & udpDebug)
    {
      myDebug[debugPort]->println(F("Contents:"));
      myDebug[debugPort]->println(PacketBuffer);
    }
    udpProcess();
  }

  setLED(LED5, ledON);
  if(runBonjour >= runBonjourTimeout)
  {
    if(setDebug & udpDebug)
    {
      myDebug[debugPort]->println(F("EthernetBonjour.run()"));
    }
    
    if(setDebug & ethDebug) // display Process Command Letter
    {
      lcd[7]->setCursor(0, 3);
      lcd[7]->print(F("EthernetBonjour.run "));
    }

    runBonjour = 0;
    EthernetBonjour.run();
  }
  setLED(LED5, ledOFF);
  
  if(timer > (millis() + 5000)) // in case of rollover
  {
    timer = millis();
  }

  updateChipStatus(chipX);
  chipX++;
  if(chipX >= maxChips)
  {
    chipX = 0;
    glcd1WUpdate();
    lcd1wUpdate();
  }
  timer = millis();
  
  updateActions(actionsCnt);    
  actionsCnt++;
  if(actionsCnt >= maxActions)
  {
    actionsCnt = 0;
    glcd1WUpdate();
    lcd1wUpdate();
  }


  updatePIDs(pidCnt);
  pidCnt++;
  if(pidCnt >= maxPIDs)
  {
    pidCnt = 0;
    glcd1WUpdate();
    lcd1wUpdate();
  }


  if(setDebug & udpTimerDebug && udpTimer > 2000) // Time since Last UDP command
  {
    myDebug[debugPort]->print(F("Time Since Last UDP Command - "));
    sprintf(lcdStrBuf, "%ld", (uint32_t) udpTimer);
    lcdCenterStr((char *) lcdStrBuf, lcdChars);
    lcd[7]->setCursor(0, 3);
    lcd[7]->print(lcdStr);
    myDebug[debugPort]->print(lcdStrBuf);
    myDebug[debugPort]->println(F(" milliseconds"));
  }
  
/*
  if(udpTimer >= udpTimerMax) // Reset if no UDP activity for more than 1 hour and 
  {
    MasterStop();
    softReset();
  }
*/

  checkMasterStop();
  checkForReset();
// end loop()
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

void lcdRightStr(char *str, uint8_t len)
{
  if((strlen(str) >= len) || (strlen(str) == len))
  {
    memcpy(&lcdStr[0], str, lcdCols);
    lcdStr[lcdCols+1] = 0x00;
    if(setDebug & lcdDebug)
    {
      if(strlen(str) > len)
      {
        myDebug[debugPort]->println(F("String was too long for the LCD display, truncating the String"));
      }else if(strlen(str) == len){
      myDebug[debugPort]->println(F("String was exactly right for the LCD display"));
      }
    }
  }else{ // length is less than len
    memcpy(&lcdStr[0], str, strlen(str));
    for(uint8_t x = strlen(str); x < lcdCols; x++)
       lcdStr[x] = 0x20; // pad with spaces
     lcdStr[lcdCols] = 0x00; //terminate the string
  }
  
  if(setDebug & lcdDebug)
  {
    myDebug[debugPort]->print(F("Returning lcdRightStr = \""));
    myDebug[debugPort]->print(lcdStr);
    myDebug[debugPort]->println(F("\""));
  }
}

void lcdCenterStr(char *str, uint8_t len)
{
  uint8_t lcdPad, x;
  for(x=0; x<=len; x++) lcdStr[x] = 0x20; // fill lcdStr with spaces
  lcdStr[x] = 0x00; // terminate the string
  
  if(strlen(str) > len)
  {
    strcpy(lcdStr, "  ERROR - TOO LONG  ");
    if(setDebug & lcdDebug)
    {
      myDebug[debugPort]->println(F("String was too long for the LCD display"));
    }
  }else if(strlen(str) == len){
    strcpy(lcdStr, str);
    if(setDebug & lcdDebug)
    {
      myDebug[debugPort]->println(F("String was exactly right for the LCD display"));
    }
  }else{
    
    if(setDebug & lcdDebug)
    {
      myDebug[debugPort]->print(F("Input String = \""));
      myDebug[debugPort]->print(str);
      myDebug[debugPort]->println(F("\""));
      myDebug[debugPort]->print(F("strlen(str) = "));
      myDebug[debugPort]->print(strlen(str));
      myDebug[debugPort]->println(F(" bytes"));
    }

    lcdPad = (len - strlen(str)) / 2;
    if(setDebug & lcdDebug)
    {
      myDebug[debugPort]->print(F("lcdPad = "));
      myDebug[debugPort]->println(lcdPad);
    }

    memcpy(&lcdStr[lcdPad], str, strlen(str));
    if(setDebug & lcdDebug)
    {
      myDebug[debugPort]->println(F("String was smaller than the LCD Display"));
      myDebug[debugPort]->print(F("lcdStr = \""));
      myDebug[debugPort]->print(lcdStr);
      myDebug[debugPort]->println(F("\""));
    }
  }
}

void checkMasterStop(void)
{
  if(setDebug & resetDebug)
  { 
    myDebug[debugPort]->println(F("Checking Master Stop"));
  }
  elapsedMillis MSTimer = 0;
  while(digitalRead(hwMasterStopPin) == LOW)
  {
    if(MSTimer >= 1000)
    {
      if(setDebug & resetDebug)
      { 
        myDebug[debugPort]->println(F("Executing Master Stop"));
      }
      MasterStop();
      break;
    }
  }
}

void checkForReset(void)
{
  if(setDebug & resetDebug)
  { 
    myDebug[debugPort]->println(F("Checking Reset"));
  }
  elapsedMillis resetTimer = 0;
  while(digitalRead(chipResetPin) == LOW)
  {
    if(resetTimer >= 1000)
    {
      if(setDebug & resetDebug)
      { 
        myDebug[debugPort]->println(F("Executing Reset"));
      }
      MasterStop();
      softReset();
      break;
    }
  }
}



#ifdef __cplusplus
extern "C" {
#endif
  void startup_early_hook() {
    WDOG_TOVALL = 0x93E0; // The next 2 lines sets the time-out value. (5 Minutes) This is the value that the watchdog timer compare itself to.
    WDOG_TOVALH = 0x0004;
    WDOG_STCTRLH = (WDOG_STCTRLH_ALLOWUPDATE | WDOG_STCTRLH_WDOGEN | WDOG_STCTRLH_WAITEN | WDOG_STCTRLH_STOPEN); // Enable WDG
    //WDOG_PRESC = 0; // prescaler 
  }
#ifdef __cplusplus
}
#endif
