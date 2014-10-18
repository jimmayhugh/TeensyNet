/********************

TeensyNet.ino

Version 0.0.45
Last Modified 10/17/2014
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
This code requires a change in the EthernetUDP.h library file to work properly. This is due primarily to the limited amount of
RAM that's available in the Arduino series versus the Teensy.

------------------------------------------
/arduino/libraries/Ethernet/EthernetUDP.h:
replace:

#define UDP_TX_PACKET_MAX_SIZE 24

with:

#if defined(__MK20DX256__) || defined(__MK20DX128__)
#define UDP_TX_PACKET_MAX_SIZE 2048
#else
#define UDP_TX_PACKET_MAX_SIZE 24
#endif

*********************/

#define USEVGAONLY 1 // if set to 1, use VGA Values ONLY, otherwise use VGA or RGB
#define USESERIAL2 1 // if set to 1, use Serial2 with FTDI for debugging

#include <PID_v1.h>
#include <math.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <errno.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>  // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <EthernetBonjour.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <t3mac.h>
#include <FastWire.h>
#include <Teensy_MCP23017.h>
#include <Teensy_RGBLCDShield.h>
#include <TeensyNetGLCD.h> // TeensyGLCD structures and commands
#include <I2CEEPROMAnything.h>
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
    digitalWrite(x, HIGH);   // turn them all off
  }

  Wire.begin();
#if __MK20DX256__ 
  Wire1.begin();
#endif


  Serial.begin(baudRate);
#if USESERIAL2 == 1
  // reassign pins 26 and 31 to use the ALT3 configuration
  // which makes them  Serial port 2 Rx(26) and Tx(31)
  CORE_PIN26_CONFIG = PORT_PCR_MUX(3);
  CORE_PIN31_CONFIG = PORT_PCR_MUX(3);
  Serial2.begin(baudRate);
  Stream &myDebug = Serial2;
#else
  Stream &myDebug = Serial;
#endif

  
  pinMode(hwMasterStopPin, INPUT_PULLUP); // set to monitor Master Stop switch
  pinMode(chipResetPin, INPUT_PULLUP); // set to monitor Reset switch
    
  delay(3000);
  
  Serial.print(F("Serial Debug starting at "));
  Serial.print(baudRate);
  Serial.println(F(" baud"));
  Serial.print(F("UDP_TX_PACKET_MAX_SIZE = "));
  Serial.println(UDP_TX_PACKET_MAX_SIZE);

  myDebug.print(F("Serial Debug starting at "));
  myDebug.print(baudRate);
  myDebug.println(F(" baud"));
  myDebug.print(F("UDP_TX_PACKET_MAX_SIZE = "));
  myDebug.println(UDP_TX_PACKET_MAX_SIZE);

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
    myDebug.print(F("i2cEeResult = 0x"));
    myDebug.println(i2cEeResult, HEX);
  }
  
  I2CEEPROM_readAnything(I2CEEPROMipAddr, i2cIPResult, I2C0x50);
  
//  if(setDebug & eepromDebug)
//  { 
    myDebug.print(F("i2cipResult = "));
    myDebug.print(i2cIPResult[0]);
    myDebug.print(F(","));
    myDebug.print(i2cIPResult[1]);
    myDebug.print(F(","));
    myDebug.print(i2cIPResult[2]);
    myDebug.print(F(","));
    myDebug.println(i2cIPResult[3]);
//  }
#endif // STATIC_IP

  I2CEEPROM_readAnything(I2CEEPROMbjAddr, bonjourNameBuf, I2C0x50);
  
  if(bonjourNameBuf[0] >= 0x20 && bonjourNameBuf[0] <= 0x7a)
  { 
    if(setDebug & eepromDebug)
    { 
      myDebug.print(F("bonjourNameBuf = "));
      myDebug.println(bonjourNameBuf);
    }
  }else{
    for(int x = 0; x < chipNameSize; x++)
    {
      bonjourNameBuf[x] = 0x0;
    }
    if(setDebug & eepromDebug)
    { 
      myDebug.println(F("bonjourNameBuf Cleared"));
    }
  }

  lcd[7]->clear();
  lcd[7]->home();
  
  if(i2cEeResult != 0x55)
  {
    if(setDebug & eepromDebug)
    { 
       myDebug.println(F("No I2CEEPROM Data"));
    }

    lcd[7]->print(F(" No I2CEEPROM Data  "));
  
    i2cEepromReady = FALSE;
    findChips();
    saveStructures();
  }else{

    if(setDebug & eepromDebug)
    { 
      myDebug.println(F("Getting EEPROM Data"));
    }

    lcd[7]->print(F("Valid I2CEEPROM Data"));
    
    I2CEEPROM_readAnything(I2CEEPROMccAddr, chipCnt, I2C0x50);
    
    if(setDebug & eepromDebug)
    { 
      myDebug.print(F("chipCnt at I2CEEPROMccAddr =  "));
      myDebug.println(chipCnt);
    }

    readStructures();
    if(setDebug & eepromDebug)
    { 
      myDebug.println(F("I2CEEPROM Data Read Completed"));
    }

    lcd[7]->setCursor(0, 3);
    lcd[7]->print(F("I2CEEPROM Read Done "));
  
    i2cEepromReady = TRUE;
    
  }
  
  if(setDebug & eepromDebug)
  { 
    myDebug.print( (sizeof(chipStruct) / sizeof(byte) ) * maxChips);
    myDebug.println(F(" bytes in chip structure array"));
    myDebug.print( (sizeof(glcd1wStruct) / sizeof(byte) ) * maxGLCDs);
    myDebug.println(F(" bytes in glcd structure array"));
    myDebug.print( (sizeof(chipActionStruct) / sizeof(byte) ) *maxActions);
    myDebug.println(F(" bytes in action structure array"));
    myDebug.print( (sizeof(chipPIDStruct) / sizeof(byte) ) *maxPIDs);
    myDebug.println(F(" bytes in pid structure Array"));
  }

  delay(3000);
  
  if(setDebug & udpDebug)
  {
    myDebug.println(F("Configuring IP"));
  }

  lcd[7]->clear();
  lcd[7]->home();
  lcd[7]->print(F("   Configuring IP   "));

  // start the Ethernet and UDP:
  read_mac();
  
  if(setDebug & udpDebug)
  {
    myDebug.print(F("MAC Address = "));
    print_mac();
    myDebug.println();
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
        myDebug.println(F("Ethernet,begin() failed - Resetting TeensyNet"));
        delay(1000);
        softReset();
      }else{
        myDebug.println(F("Ethernet,begin() dhcp success"));
        Udp.begin(localPort);

        myDebug.print(F("My IP address: "));
        myDebug.println(Ethernet.localIP());
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
      myDebug.println(F("Ethernet,begin() from I2CEEPROM success"));
      myDebug.print(F("My IP address: "));
      myDebug.println(Ethernet.localIP());
      break;
    }
  }
#endif
// start Bonjour service
//  if(EthernetBonjour.begin("TeensyNetTURD"))
  digitalWrite(LED5, LOW);
  if(EthernetBonjour.begin(bonjourNameBuf))
  {
    myDebug.println(F("Bounjour Service started"));
    sprintf(bonjourBuf, "%s._discover", bonjourNameBuf);
    EthernetBonjour.addServiceRecord(bonjourBuf, localPort, MDNSServiceUDP);
    I2CEEPROM_writeAnything(I2CEEPROMbjAddr, bonjourNameBuf, I2C0x50);
  }else{
    myDebug.println(F("Bounjour Service failed"));
  }
  EthernetBonjour.run();
  digitalWrite(LED5, HIGH);

// send startup data to status LCD (I2C address 0x27) if available  
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

#if USESERIAL2 == 1
  Stream &myDebug = Serial2;
#else
  Stream &myDebug = Serial;
#endif


void loop()
{
  packetSize = Udp.parsePacket();
  if(packetSize)
  {
    if(setDebug & udpDebug)
    {
      myDebug.print(F("\nLoop:\nReceived packet of size: "));
      myDebug.println(packetSize);
    }
    for(int pbSize = packetSize; pbSize < UDP_TX_PACKET_MAX_SIZE; pbSize++)
    {
      PacketBuffer[pbSize] = 0; // clear the remainder of the buffer
    }
    if(setDebug & udpDebug)
    {
      myDebug.print("From ");
      IPAddress remote = Udp.remoteIP();
      for (int i =0; i < 4; i++)
      {
        myDebug.print(remote[i], DEC);
        if (i < 3)
        {
          myDebug.print(".");
        }
      }
      myDebug.print(", port ");
      myDebug.println(Udp.remotePort());
    }
    // read the packet into packetBufffer
    Udp.read(PacketBuffer,UDP_TX_PACKET_MAX_SIZE);
    if(setDebug & udpDebug)
    {
      myDebug.println(F("Contents:"));
      myDebug.println(PacketBuffer);
    }
    udpProcess();
  }

  digitalWrite(LED5, LOW);
  if(runBonjour >= runBonjourTimeout)
  {
    if(setDebug & udpDebug)
    {
      myDebug.println(F("EthernetBonjour.run()"));
    }
    
    if(setDebug & ethDebug) // display Process Command Letter
    {
      lcd[7]->setCursor(0, 3);
      lcd[7]->print(F("EthernetBonjour.run "));
    }

    runBonjour = 0;
    EthernetBonjour.run();
  }
  digitalWrite(LED5, HIGH);
  
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
  if(chipX >= maxChips)
  {
    chipX = 0;
    glcd1WUpdate();
  }
  timer = millis();
  
  updateActions(actionsCnt);    
  actionsCnt++;
  if(actionsCnt >= maxActions)
  {
    actionsCnt = 0;
    glcd1WUpdate();
  }


  updatePIDs(pidCnt);
  pidCnt++;
  if(pidCnt >= maxPIDs)
  {
    pidCnt = 0;
    glcd1WUpdate();
  }


  if(setDebug & udpTimerDebug && udpTimer > 2000) // Time since Last UDP command
  {
    myDebug.print(F("Time Since Last UDP Command - "));
    sprintf(lcdStrBuf, "%ld", (uint32_t) udpTimer);
    lcdCenterStr((char *) lcdStrBuf, lcdChars);
    lcd[7]->setCursor(0, 3);
    lcd[7]->print(lcdStr);
    myDebug.print(lcdStrBuf);
    myDebug.println(F(" milliseconds"));
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
    myDebug.print(F("i2cEeResult = 0x"));
    myDebug.println(i2cEeResult, HEX);
  }
  
  if(i2cEeResult != 0x55)
  {
    if(setDebug & eepromDebug)
    { 
       myDebug.println(F("No EEPROM Data"));
    }
    i2cEepromReady = FALSE;
  }else{
    if(setDebug & eepromDebug)
    { 
       myDebug.println(F("EEPROM Data Valid"));
    }
    i2cEepromReady = TRUE;
  }
  
  if(setDebug & eepromDebug)
  {
    myDebug.println(F("Entering readStructures"));
    myDebug.print(F("I2CEEPROMchipAddr = 0x"));
    myDebug.println(I2CEEPROMchipAddr, HEX);
  }

  i2cEeResult16 = I2CEEPROM_readAnything(I2CEEPROMchipAddr, chip, I2C0x50);


  if(setDebug & eepromDebug)
  {
    myDebug.print(F("Read "));
    myDebug.print(i2cEeResult16);
    myDebug.print(F(" bytes from address Ox"));
    myDebug.println(I2CEEPROMchipAddr, HEX);
  }

  i2cEeResult16 = I2CEEPROM_readAnything(I2CEEPROMglcdAddr, glcd1w, I2C0x50);


  if(setDebug & eepromDebug)
  {
    myDebug.print(F("Read "));
    myDebug.print(i2cEeResult16);
    myDebug.print(F(" bytes from address Ox"));
    myDebug.println(I2CEEPROMglcdAddr, HEX);
  }

  if(setDebug & eepromDebug)
  {
    myDebug.print(F("I2CEEPROMactionAddr = 0x"));
    myDebug.println(I2CEEPROMactionAddr, HEX);
  }

  i2cEeResult16 = I2CEEPROM_readAnything(I2CEEPROMactionAddr, action,  I2C0x50);

  if(setDebug & eepromDebug)
  {
    myDebug.print(F("Read "));
    myDebug.print(i2cEeResult16);
    myDebug.print(F(" bytes from address Ox"));
    myDebug.println(I2CEEPROMactionAddr, HEX);
  }

  if(setDebug & eepromDebug)
  {
    myDebug.print(F("I2CEEPROMpidAddr = 0x"));
    myDebug.println(I2CEEPROMpidAddr, HEX);
  }

  i2cEeResult16 = I2CEEPROM_readAnything(I2CEEPROMpidAddr, ePID,  I2C0x50);

  if(setDebug & eepromDebug)
  {
    myDebug.print(F("Read "));
    myDebug.print(i2cEeResult16);
    myDebug.print(F(" bytes from address Ox"));
    myDebug.println(I2CEEPROMpidAddr, HEX);
    myDebug.println(F(" Completed"));
    myDebug.println(F("Exiting readStructures"));
    myDebug.println(F("Chip Structure"));
    displayStructure((byte *)(uint32_t) &chip, sizeof(chip), sizeof(chipStruct));
    myDebug.println(F("GLCD Structure"));
    displayStructure((byte *)(uint32_t) &glcd1w, sizeof(glcd1w), sizeof(glcd1wStruct));
    myDebug.println(F("Action Structure"));
    displayStructure((byte *)(uint32_t) &action, sizeof(action), sizeof(chipActionStruct));
    myDebug.println(F("PID Structure"));
    displayStructure((byte *)(uint32_t) &ePID, sizeof(ePID), sizeof(chipPIDStruct));
  }

  pidSetup();
}


void saveStructures(void)
{  
  if(setDebug & eepromDebug)
  {
    myDebug.println(F("Entering saveStructures"));
    myDebug.print(F("I2CEEPROMchipAddr = 0x"));
    myDebug.println(I2CEEPROMchipAddr, HEX);
  }
  I2CEEPROM_writeAnything(I2CEEPROMccAddr, chipCnt, I2C0x50);
  I2CEEPROM_writeAnything(I2CEEPROMglAddr, numGLCDs, I2C0x50);
  I2CEEPROM_writeAnything(I2CEEPROMidAddr, I2CEEPROMidVal, I2C0x50);
  i2cEeResult16 = I2CEEPROM_writeAnything(I2CEEPROMchipAddr, chip, I2C0x50);
  if(setDebug & eepromDebug)
  {
    myDebug.print(F("Wrote "));
    myDebug.print(i2cEeResult);
    myDebug.print(F(" bytes to address Ox"));
    myDebug.print(I2CEEPROMchipAddr, HEX);
    myDebug.print(F(" from address 0x"));
    uint32_t chipStructAddr = (uint32_t) &chip[0];
    myDebug.println(chipStructAddr, HEX);
  }

  i2cEeResult16 = I2CEEPROM_writeAnything(I2CEEPROMglcdAddr, glcd1w, I2C0x50);
  if(setDebug & eepromDebug)
  {
    myDebug.print(F("Wrote "));
    myDebug.print(i2cEeResult);
    myDebug.print(F(" bytes to address Ox"));
    myDebug.print(I2CEEPROMglcdAddr, HEX);
    myDebug.print(F(" from address 0x"));
    uint32_t glcd1wStructAddr = (uint32_t) &glcd1w[0];
    myDebug.println(glcd1wStructAddr, HEX);
  }

  if(setDebug & eepromDebug)
  {
    myDebug.print(F("I2CEEPROMactionAddr = 0x"));
    myDebug.println(I2CEEPROMactionAddr, HEX);
  }
  i2cEeResult16 = I2CEEPROM_writeAnything(I2CEEPROMactionAddr, action, I2C0x50);

  if(setDebug & eepromDebug)
  {
    myDebug.print(F("Wrote "));
    myDebug.print(i2cEeResult16);
    myDebug.print(F(" bytes to address Ox"));
    myDebug.println(I2CEEPROMactionAddr, HEX);
  }

  if(setDebug & eepromDebug)
  {
    myDebug.print(F("I2CEEPROMpidAddr = 0x"));
    myDebug.println(I2CEEPROMpidAddr, HEX);
  }
  i2cEeResult16 = I2CEEPROM_writeAnything(I2CEEPROMpidAddr, ePID, I2C0x50);

  if(setDebug & eepromDebug)
  {
    myDebug.print(F("Wrote "));
    myDebug.print(i2cEeResult16);
    myDebug.print(F(" bytes to address Ox"));
    myDebug.println(I2CEEPROMpidAddr, HEX);
    myDebug.println(F(" Completed - Displaying chip Structures"));
    myDebug.println(F("Chip Structure"));
    displayStructure((byte *)(uint32_t) &chip, sizeof(chip), sizeof(chipStruct));
    myDebug.println(F("GLCD Structure"));
    displayStructure((byte *)(uint32_t) &glcd1w, sizeof(glcd1w), sizeof(glcd1wStruct));
    myDebug.println(F("Action Structure"));
    displayStructure((byte *)(uint32_t) &action, sizeof(action), sizeof(chipActionStruct));
    myDebug.println(F("PID Structure"));
    displayStructure((byte *)(uint32_t) &ePID, sizeof(ePID), sizeof(chipPIDStruct));
    myDebug.println(F("Exiting saveStructures"));
  }
  I2CEEPROM_readAnything(I2CEEPROMidAddr, i2cEeResult, I2C0x50);
  
  if(setDebug & eepromDebug)
  { 
    myDebug.print(F("i2cEeResult = 0x"));
    myDebug.println(i2cEeResult, HEX);
  }
  
  if(i2cEeResult != 0x55)
  {
    if(setDebug & eepromDebug)
    { 
       myDebug.println(F("EEPROM Data Erased"));
    }
    i2cEepromReady = FALSE;
  }else{
    if(setDebug & eepromDebug)
    { 
       myDebug.println(F("EEPROM Data Valid"));
    }
    i2cEepromReady = TRUE;
  }
  
}

void displayStructure(byte *addr, int xSize, int ySize)
{
  int x, y;
  myDebug.print(F("0x"));
  myDebug.print((uint32_t)addr, HEX);
  myDebug.print(F(": ")); 
  for(x = 0, y = 0; x < xSize; x++)
  {
    if(addr[x] >=0 && addr[x] <= 15)
    {
      myDebug.print(F("0x0"));
    }else{
      myDebug.print(F("0x"));
    }
    myDebug.print(addr[x], HEX);
    y++;
    if(y < ySize)
    {
      myDebug.print(F(", "));
    }else{
      y = 0;
      myDebug.println();
      myDebug.print(F("0x"));
      myDebug.print((uint32_t)addr + x + 1, HEX);
      myDebug.print(F(": ")); 
    }
  }
  myDebug.println();
  myDebug.println();
}


void updatePIDs(uint8_t pidCnt)
{
  
  if(ePID[pidCnt].pidEnabled == 1)
  {    
    // *** Start PID Loop ***
    ePID[pidCnt].pidInput = (double) ePID[pidCnt].tempPtr->chipStatus;

    if(setDebug & pidDebug)
    {
      myDebug.println(F("Entering updatePIDs"));
      myDebug.print(F("PID #"));
      myDebug.println(pidCnt);
      myDebug.print(F("ePID["));
      myDebug.print(pidCnt);
      myDebug.print(F("].pidInput = "));
      myDebug.println((double) ePID[pidCnt].pidInput);
      myDebug.print(F("ePID["));
      myDebug.print(pidCnt);
      myDebug.print(F("].pidKp = "));
      myDebug.println(ePID[pidCnt].pidKp);
      myDebug.print(F("ePID["));
      myDebug.print(pidCnt);
      myDebug.print(F("].pidKi = "));
      myDebug.println(ePID[pidCnt].pidKi);
      myDebug.print(F("ePID["));
      myDebug.print(pidCnt);
      myDebug.print(F("].pidKd = "));
      myDebug.println(ePID[pidCnt].pidKd);
      myDebug.print(F("ePID["));
      myDebug.print(pidCnt);
      myDebug.print(F("].pidDirection = "));
      myDebug.println(ePID[pidCnt].pidDirection);
      myDebug.print(F("ePID["));
      myDebug.print(pidCnt);
      myDebug.print(F("].pidWindowStartTime = "));
      myDebug.println((uint32_t) ePID[pidCnt].pidwindowStartTime);
      myDebug.print(F("millis() = "));
      myDebug.println((uint32_t) millis());
    }
  
    if(ePID[pidCnt].myPID->Compute())
    {
      if(setDebug & pidDebug)
      {
        myDebug.println(F("Compute() returned TRUE"));
      }
    }else{
      if(setDebug & pidDebug)
      {
        myDebug.println(F("Compute() returned FALSE"));
      }
    }

    uint32_t now = millis();
    
    if(setDebug & pidDebug)
    {
      myDebug.print(F("now - ePID[pidCnt].pidwindowStartTime = "));
      myDebug.println(now - ePID[pidCnt].pidwindowStartTime);
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
        myDebug.print(F("ePID["));
        myDebug.print(pidCnt);
        myDebug.print(F("].pidOutPut = "));
        myDebug.println((double) ePID[pidCnt].pidOutput);
        myDebug.print(F("now = "));
        myDebug.println(now);
        myDebug.print(F("ePID["));
        myDebug.print(pidCnt);
        myDebug.print(F("].pidwindowStartTime = "));
        myDebug.println((double) ePID[pidCnt].pidwindowStartTime);
        myDebug.print(F("now - ePID["));
        myDebug.print(pidCnt);
        myDebug.print(F("].pidwindowStartTime = "));
        myDebug.println((double) now - ePID[pidCnt].pidwindowStartTime);
  
        myDebug.print((double) ePID[pidCnt].pidOutput);
        
        if(ePID[pidCnt].pidOutput > now - ePID[pidCnt].pidwindowStartTime)
        {
          myDebug.print(F(" > "));
        }else{
          myDebug.print(F(" < "));
        }
        myDebug.println((double) now - ePID[pidCnt].pidwindowStartTime);
      }

    if(ePID[pidCnt].pidOutput > now - ePID[pidCnt].pidwindowStartTime)
    {
      if(setDebug & pidDebug)
      {
        myDebug.println(F("Turning Switch ON"));
      }
      actionSwitchSet((uint8_t *) &ePID[pidCnt].switchPtr->chipAddr, ds2406PIOAon);
    }else{
      if(setDebug & pidDebug)
      {
        myDebug.println(F("Turning Switch OFF"));
      }
      actionSwitchSet((uint8_t *) &ePID[pidCnt].switchPtr->chipAddr, ds2406PIOAoff);
    }
  // *** End PID Loop ***

    if(setDebug & pidDebug)
      {
        myDebug.print(F("ePID["));
        myDebug.print(pidCnt);
        myDebug.print(F("].pidOutput = "));
        myDebug.println((double) ePID[pidCnt].pidOutput);
        myDebug.println(F("Exiting updatePIDs"));
      }

  }else{
    ePID[pidCnt].myPID->SetMode(MANUAL);
  }
}

void glcd1WUpdate(void)
{
uint8_t color;
char tempStr[7]    = "      ";
char actionStr[16] = "               ";
char switchStr[7] = "UNUSED";

  if(setDebug & glcd1WLED)
    digitalWrite(LED1, LOW);
    
  switch(glcd1w[glcdCnt].Addr[0])
  {
    case 0x45:
    {
//      myDebug.println(F("GLCD Found!!"));
      if(glcd1w[glcdCnt].Flags & glcdFActive)
      {
        if(setDebug & glcdSerialDebug)
        {
          myDebug.println(F("GLCD Active!!"));
        }
        switch(glcd1w[glcdCnt].Flags & (glcdFAction | glcdFChip))
        {
          case glcdFAction:
          {
            switch(glcd1w[glcdCnt].Item)
            {
              case glcdActionSetPage:
              {
                if(setDebug & glcdSerialDebug)
                {
                  myDebug.print(F("Setting Page Write to "));
                  myDebug.println(glcd1w[glcdCnt].Page);
                }
                sendCommand(dsDevice, glcdCnt, setPageWrite, 0, 0, 0, glcd1w[glcdCnt].Page, 0, 0, "", 0, 0, 0, 0, 0, 1); // Set the page to write into
                glcd1w[glcdCnt].Item += 1;
                break;
              }

              case glcdDrawRectangle:
              {
                if(setDebug & glcdSerialDebug)
                {
                  myDebug.print(F("Placing Filled Rectangle"));
                }
                if( (glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position] == NULL) | 
                    (glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->tempPtr == NULL)
                  )
                {
                  color = YELLOW;
                }else if(glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->tempPtr->chipStatus <= glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->tooCold){
                  color = BLUE;
                }else if(glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->tempPtr->chipStatus >= glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->tooHot){
                  color = RED;
                }else{
                  color = LIME;
                }

                sendCommand(dsDevice, glcdCnt, (setColorVGA | setDrawFRRect | setBackVGA), 0,
                             BLACK, color,
                             0, 0, 0, "",
                             action2x2[glcd1w[glcdCnt].Position].rectX1, action2x2[glcd1w[glcdCnt].Position].rectY1,
                             action2x2[glcd1w[glcdCnt].Position].rectX2,  action2x2[glcd1w[glcdCnt].Position].rectY2, 0, 1);
                
                glcd1w[glcdCnt].Item += 1;
                break;
              }

              case glcdPrintTempName:
              {
                if(setDebug & glcdDebug)
                {
                  sprintf(actionStr, "LCD %d - Page %d", glcdCnt, glcd1w[glcdCnt].Page);
                }else{
                  if( (glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position] == NULL) | 
                      (glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->tempPtr == NULL)
                    )
                  {
                    lcdCenterStr("UNNAMED", chipNameSize);
//                    sprintf(actionStr, "%s", "    UNNAMED    ");
                    sprintf(actionStr, "%s", lcdStr);
                  }else{
                    lcdCenterStr(glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->tempPtr->chipName, chipNameSize);
//                    sprintf(actionStr, "%s", glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->tempPtr->chipName);
                    sprintf(actionStr, "%s", lcdStr);
                  }
                }
                sendCommand(dsDevice, glcdCnt, (setColorVGA | setPrintStrXY | setFont),  action2x2[glcd1w[glcdCnt].Position].tFont,
                             0, WHITE,
                             0, 0, 0, actionStr,
                             action2x2[glcd1w[glcdCnt].Position].tX1, action2x2[glcd1w[glcdCnt].Position].tY1, 0, 0, 0, 1);
                glcd1w[glcdCnt].Item += 1;
                break;
              }

              case glcdPrintTemp:
              {
                if( (glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position] == NULL) | 
                    (glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->tempPtr == NULL)
                  )
                {
                  sprintf(tempStr, "%s", "------");
                }else{
                  sprintf(tempStr, "%4d""%c", glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->tempPtr->chipStatus, '.');
                }
                lcdCenterStr(tempStr, 6);
                sprintf(tempStr, "%s", lcdStr);
                sendCommand(dsDevice, glcdCnt, (setColorVGA | setPrintStrXY | setFont),  action2x2[glcd1w[glcdCnt].Position].vFont,
                             0, WHITE,
                             0, 0, 0, tempStr,
                             action2x2[glcd1w[glcdCnt].Position].vX1, action2x2[glcd1w[glcdCnt].Position].vY1, 0, 0, 0, 1);
                glcd1w[glcdCnt].Item += 1;
                break;
              }

              case glcdPrintS1Name:
              {
                if(setDebug & glcdDebug)
                {
                  sprintf(actionStr, "Page %d - Pos %d", glcd1w[glcdCnt].Page, glcd1w[glcdCnt].Position);
                }else{
                  if( (glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position] == NULL) | 
                      (glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->tcPtr == NULL)
                    )
                  {
                    lcdCenterStr("UNNAMED", chipNameSize);
//                    sprintf(actionStr, "%s", "    UNNAMED    ");
                    sprintf(actionStr, "%s", lcdStr);
                  }else{
                    lcdCenterStr(glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->tcPtr->chipName, chipNameSize);
//                    sprintf(actionStr, "%s", glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->tempPtr->chipName);
                    sprintf(actionStr, "%s", lcdStr);
                  }
                }
                sendCommand(dsDevice, glcdCnt, (setColorVGA | setPrintStrXY | setFont),  action2x2[glcd1w[glcdCnt].Position].s1Font,
                             0, WHITE,
                             0, 0, 0, actionStr,
                             action2x2[glcd1w[glcdCnt].Position].s1X1, action2x2[glcd1w[glcdCnt].Position].s1Y1, 0, 0, 0, 1);
                glcd1w[glcdCnt].Item += 1;
                break;
              }

              case glcdPrintS1Val:
              {
                if( (glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position] == NULL) | 
                    (glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->tcPtr == NULL)
                  )
                {
                  sprintf(switchStr, "%s", "UNUSED");
                }else{
                  if(glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->tcPtr->chipStatus == 'N')
                  {
                    lcdCenterStr("ON", sizeof(switchStr));
                  }else{
                   lcdCenterStr("OFF", sizeof(switchStr));
                  }
                  sprintf(switchStr, "%s", lcdStr);
                }

                sendCommand(dsDevice, glcdCnt, (setColorVGA | setPrintStrXY | setFont),  action2x2[glcd1w[glcdCnt].Position].s2Font,
                             0, WHITE,
                             0, 0, 0, switchStr,
                             action2x2[glcd1w[glcdCnt].Position].s2X1, action2x2[glcd1w[glcdCnt].Position].s2Y1, 0, 0, 0, 1);
                glcd1w[glcdCnt].Item += 1;
                break;
              }

              case glcdPrintS2Name:
              {
                if(setDebug & glcdDebug)
                {
                  sprintf(actionStr, "Page %d - Pos %d", glcd1w[glcdCnt].Page, glcd1w[glcdCnt].Position);
                }else{
                  if( (glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position] == NULL) | 
                      (glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->thPtr == NULL)
                    )
                  {
                    lcdCenterStr("UNNAMED", chipNameSize);
//                    sprintf(actionStr, "%s", "    UNNAMED    ");
                    sprintf(actionStr, "%s", lcdStr);
                  }else{
                    lcdCenterStr(glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->thPtr->chipName, chipNameSize);
//                    sprintf(actionStr, "%s", glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->tempPtr->chipName);
                    sprintf(actionStr, "%s", lcdStr);
                  }
                }
                sendCommand(dsDevice, glcdCnt, (setColorVGA | setPrintStrXY | setFont),  action2x2[glcd1w[glcdCnt].Position].s3Font,
                             0, WHITE,
                             0, 0, 0, actionStr,
                             action2x2[glcd1w[glcdCnt].Position].s3X1, action2x2[glcd1w[glcdCnt].Position].s3Y1, 0, 0, 0, 1);
                glcd1w[glcdCnt].Item += 1;
                break;
              }

              case glcdPrintS2Val:
              {
                if( (glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position] == NULL) | 
                    (glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->thPtr == NULL)
                  )
                {
                  sprintf(switchStr, "%s", "UNUSED");
                }else{
                  if(glcd1w[glcdCnt].Action[glcd1w[glcdCnt].Position]->thPtr->chipStatus == 'N')
                  {
                    lcdCenterStr("ON", sizeof(switchStr));
                  }else{
                   lcdCenterStr("OFF", sizeof(switchStr));
                  }
                  sprintf(switchStr, "%s", lcdStr);
                }

                sendCommand(dsDevice, glcdCnt, (setColorVGA | setPrintStrXY | setFont),  action2x2[glcd1w[glcdCnt].Position].s4Font,
                             0, WHITE,
                             0, 0, 0, switchStr,
                             action2x2[glcd1w[glcdCnt].Position].s4X1, action2x2[glcd1w[glcdCnt].Position].s4Y1, 0, 0, 0, 1);
                glcd1w[glcdCnt].Item = 0;
                glcd1w[glcdCnt].Position += 1;                
                break;
              }

/*
              case :
              {
                break;
              }
*/
              default:
              {
                break;
              }
            }
            
            switch(glcd1w[glcdCnt].Position)
            {
              case 0:
              case 1:
              case 2:
              case 3:
              {
                break; // do nothing
              }
              
              case 4:
              {
                glcd1w[glcdCnt].Position = 0;
                sendCommand(dsDevice, glcdCnt, setPageDisplay, 0, 0, 0, glcd1w[glcdCnt].Page, 0, 0, "", 0, 0, 0, 0, 0, 1); // Set the page to write into
                glcd1w[glcdCnt].Page     += 1;  // set net page to write into
                glcdCnt += 1; // on to next display
                break;
              }
            }
            
            switch(glcd1w[glcdCnt].Page)
            {
              case 0:
              case 1:
              case 2:
              case 3:
              case 4:
              case 5:
              case 6:
              case 7:
              {
                break; //do nothing
              }
              
              case 8: // reset everything and go to the next page
              {
                glcd1w[glcdCnt].Item     = 0;
                glcd1w[glcdCnt].Position = 0;
                glcd1w[glcdCnt].Page     = 0;
                glcdCnt += 1;
                break;
              }
            }
            
            break;
          }

          case glcdFChip:
          {
            break;
          }

          default:
          {
            break;
          }
        }
      }else{
        glcdCnt += 1;
        if( glcdCnt > 7)
        {
          glcdCnt = 0;
        }
      }

      break;
    }
    
    case 0:
    default:
    {
      
      glcdCnt = 0; // reset the count and start over
      break;
    }
  }
  if(setDebug & glcd1WLED)
    digitalWrite(LED1, HIGH);  
}

void writeToGLCD(uint8_t page, uint8_t x)
{
  char str1[displayStrSize+1];

  sendCommand(dsDevice, x, (setInitL | setPageWrite | setLCDon), 0, 
                   0, 0,
                   page, 0, 0, "",
                   0, 0, 0, 0, 0, 1);

  sprintf(str1, "Display %d", x);
  sendCommand(dsDevice, x, (setBackVGA | setColorVGA | setFont | setPrintStr ), UBUNTUBOLD, 
                   BLACK,  WHITE,
                   0, 0, 0, str1,
                   0, 0, 0, 0, 0, 1);

  sprintf(str1, "%0X, %0X, %0X, %0X, %0X, %0X, %0X, %0X", 
          glcd1w[x].Addr[0],glcd1w[x].Addr[1],glcd1w[x].Addr[2],glcd1w[x].Addr[3],
          glcd1w[x].Addr[4],glcd1w[x].Addr[5],glcd1w[x].Addr[6],glcd1w[x].Addr[7]);
  sendCommand(dsDevice, x, (setPrintStr), 0, 
                   0, 0,
                   0, 2, 0, str1,
                   0, 0, 0, 0, 0, 1);

  sprintf(str1, "Page %d - %ld", page, glcdCnt32);
  glcdCnt32++;
  sendCommand(dsDevice, x, (setPrintStr), 0, 
                   0, 0,
                   0, 4, 0, str1,
                   0, 0, 0, 0, 0, 1);

  sendCommand(dsDevice, x, (setPageDisplay), 0, 
                   0, 0,
                   page, 0, 0, "",
                   0, 0, 0, 0, 0, 1);

}


void updateGLCD1W(int cnt, uint8_t x)
{
  for(uint8_t y = 0; y < chipAddrSize; y++)
  {
    glcd1w[cnt].Addr[y] = chip[x].chipAddr[y]; // move the address to the GLCD 1-wire array
    chip[x].chipAddr[y] = 0; // reset the array
  }

  char str1[displayStrSize+1];

// initialize the glcd structure and device

  sendCommand(dsDevice, x, (setInitL | setPageWrite), 0,
              0, 0,
              0, 0, 0, "",
              0, 0, 0, 0, 0, 1);

  sprintf(str1, "Display %d", cnt);
  sendCommand(dsDevice, x, (setBackVGA | setColorVGA | setFont | setPrintStr ), UBUNTUBOLD, 
                   BLACK, WHITE,
                   0, 0, 0, str1,
                   0, 0, 0, 0, 0, 1);

  sprintf(str1, "%0X, %0X, %0X, %0X, %0X, %0X, %0X, %0X", 
          glcd1w[cnt].Addr[0],glcd1w[cnt].Addr[1],glcd1w[cnt].Addr[2],glcd1w[cnt].Addr[3],
          glcd1w[cnt].Addr[4],glcd1w[cnt].Addr[5],glcd1w[cnt].Addr[6],glcd1w[cnt].Addr[7]);
  sendCommand(dsDevice, x, (setPrintStr), 0, 
                   0, 0,
                   0, 2, 0, str1,
                   0, 0, 0, 0, 0, 1);

  sendCommand(dsDevice, x, (setPageDisplay), 0, 
                   0, 0,
                   0, 0, 0, "",
                   0, 0, 0, 0, 0, 1);

  if(setDebug & glcdSerialDebug)
  {
    myDebug.print(F("GLCD "));
    myDebug.print(cnt);
    myDebug.print(F(" Address 0x"));
    myDebug.print((uint32_t) &(glcd1w[cnt].Addr), HEX);
    myDebug.print(F(" = {"));

    for( int i = 0; i < chipAddrSize; i++)
    {
      if(glcd1w[cnt].Addr[i]>=0 && glcd1w[cnt].Addr[i]<16)
      {
        myDebug.print(F("0x0"));
      }else{
        myDebug.print(F("0x"));
      }
      myDebug.print(glcd1w[cnt].Addr[i], HEX);
      if(i < 7){myDebug.print(F(","));}
    }
    myDebug.println(F("}"));
  }
}


void findChips(void)
{
 int cntx = 0, cmpCnt, cmpArrayCnt, dupArray = 0, cnty, glcdCnt = 0;

  digitalWrite(LED1, LOW); //start DSO sync

  ds.reset_search();
  delay(250);
  
  for(cnty = 0; cnty < maxChips; cnty++) // clear all chipnames
  {
    strcpy(chip[cnty].chipName, "");
  }
  
  while (ds.search(chip[cntx].chipAddr))
  {
    if((chip[cntx].chipAddr[0] == dsGLCD) || (chip[cntx].chipAddr[0] == dsGLCDP))
    {
      if(ds.crc8(chip[cntx].chipAddr, chipAddrSize-1) != chip[cntx].chipAddr[chipAddrSize-1])
      {
        continue;
      }
      updateGLCD1W(glcdCnt, cntx);
      glcdCnt++;
      continue;
    }
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
        myDebug.print(F("Chip "));
        myDebug.print(cntx);
        myDebug.println(F(" - Duplicated Array"));
      }
      dupArray = 0;
      continue;
    }
    
    if(ds.crc8(chip[cntx].chipAddr, chipAddrSize-1) != chip[cntx].chipAddr[chipAddrSize-1])
    {
      continue;
      if(setDebug & findChipDebug)
      {
        myDebug.print(F("CRC Error - Chip "));
        myDebug.print(cntx);
        myDebug.print(F(" = "));
        myDebug.print(chip[cntx].chipAddr[chipAddrSize-1], HEX);
        myDebug.print(F(", CRC should be "));
        myDebug.println(ds.crc8(chip[cntx].chipAddr, chipAddrSize-1));
      }
    }

    if(setDebug & findChipDebug)
    {
      myDebug.print(F("Chip "));
      myDebug.print(cntx);
      myDebug.print(F(" Address 0x"));
      myDebug.print((uint32_t) &(chip[cntx].chipAddr), HEX);
      myDebug.print(F(" = {"));
      
      for( int i = 0; i < chipAddrSize; i++)
      {
        if(chip[cntx].chipAddr[i]>=0 && chip[cntx].chipAddr[i]<16)
        {
          myDebug.print(F("0x0"));
        }else{
          myDebug.print(F("0x"));
        }
        myDebug.print(chip[cntx].chipAddr[i], HEX);
        if(i < 7){myDebug.print(F(","));}
      }
      myDebug.println(F("}"));
    }
      
    cntx++;
    delay(50);
  }

  ds.reset_search();
  delay(250);
  chipCnt = cntx;
  numGLCDs = glcdCnt;
  
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
  
  if(glcdCnt < maxGLCDs)
  {
    for(;glcdCnt<maxGLCDs;glcdCnt++)
    {
      for(int y=0;y<chipAddrSize;y++)
      {
        glcd1w[glcdCnt].Addr[y]=0;
      }
    }
  }
  
  if(setDebug & findChipDebug)
  {
    myDebug.print(chipCnt);
    myDebug.print(F(" Sensor"));
    if(chipCnt == 1)
    {
      myDebug.println(F(" Detected"));
    }else{
      myDebug.println(F("s Detected"));
    }
  }  
  digitalWrite(LED1, HIGH); // end DSO sync
}

void asciiArrayToHexArray(char* result, char* addrDelim, uint8_t* addrVal)
{
  char *addrResult = NULL;
  uint16_t addrResultCnt = 0;
  
  addrResult = strtok( result, addrDelim );
  while(addrResult != NULL)
  {
    addrVal[addrResultCnt] = (uint8_t) strtol(addrResult, NULL, 16);
    
    if( (setDebug & pidDebug) || (setDebug & glcdNameUpdate) )
    {
      if(addrVal[addrResultCnt] >= 0 && addrVal[addrResultCnt] <= 9)
      {
        myDebug.print(F(" 0x0"));
      }else{
        myDebug.print(F(" 0x"));
      }
      myDebug.print(addrVal[addrResultCnt], HEX);
    }
      
    addrResultCnt++;
    addrResult = strtok( NULL, addrDelim );
   }
   
  if( (setDebug & pidDebug) || (setDebug & glcdNameUpdate) )
  {
    myDebug.println();
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
   myDebug.println(F("matchChipAddress"));
  }
  
  for(addrMatchCnt = 0, chipAddrCnt = 0; ((addrMatchCnt < chipAddrSize) || (chipAddrCnt > chipCnt)); addrMatchCnt++)
  {
    if(array[addrMatchCnt] != chip[chipAddrCnt].chipAddr[addrMatchCnt])
    {
      addrMatchCnt = 0;
      chipAddrCnt++;
      
      if(setDebug & pidDebug)
      {
        myDebug.println(chipAddrCnt);
      }
  
      continue;
    }
    
    if(setDebug & pidDebug)
    {
      myDebug.print(array[addrMatchCnt], HEX);
      myDebug.print(F(","));
    }
  }
  
  if(chipAddrCnt <= chipCnt)
  {
    if(setDebug & pidDebug)
    {
      myDebug.print(F("MATCH!! - "));
    }
  }else{

    if(setDebug & pidDebug)
    {
      myDebug.print(F("NO MATCH!! - "));
    }

    chipAddrCnt = 0xFF;
  }

  if(setDebug & pidDebug)
  {
    myDebug.println(chipAddrCnt);
  }

  return(chipAddrCnt);
}

uint8_t matchglcd1wAddress(uint8_t* array)
{
   uint8_t addrMatchCnt, glcd1wAddrCnt;
   
  if(setDebug & pidDebug)
  {
   myDebug.println(F("matchglcd1wAddress"));
  }
  
  for(addrMatchCnt = 0, glcd1wAddrCnt = 0; ((addrMatchCnt < chipAddrSize) || (glcd1wAddrCnt > glcdCnt)); addrMatchCnt++)
  {
    if(array[addrMatchCnt] != glcd1w[glcd1wAddrCnt].Addr[addrMatchCnt])
    {
      addrMatchCnt = 0;
      glcd1wAddrCnt++;
      
      if(setDebug & pidDebug)
      {
        myDebug.println(glcd1wAddrCnt);
      }
  
      continue;
    }
    
    if(setDebug & pidDebug)
    {
      myDebug.print(array[addrMatchCnt], HEX);
      myDebug.print(F(","));
    }
  }
  
  if(glcd1wAddrCnt <= glcdCnt)
  {
    if(setDebug & pidDebug)
    {
      myDebug.print(F("MATCH!! - "));
    }
  }else{

    if(setDebug & pidDebug)
    {
      myDebug.print(F("NO MATCH!! - "));
    }

    glcd1wAddrCnt = 0xFF;
  }

  if(setDebug & pidDebug)
  {
    myDebug.println(glcd1wAddrCnt);
  }

  return(glcd1wAddrCnt);
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
    myDebug.print(F("Chip Address 0x"));
    myDebug.print((uint32_t) array, HEX);
    myDebug.print(F(" = "));
  }
  
  for( int i = 0; i < chipAddrSize; i++)
  {
    rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s", "0x");    
    rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%02X",(uint8_t) array[i]);
    if(setDebug & findChipDebug)
    {
      myDebug.print(F("0x"));
      if( array[i] < 0x10 ) myDebug.print(F("0")); 
      myDebug.print((uint8_t) array[i], HEX);
      if(i < 7)myDebug.print(F(","));
    }
    if(i < 7)
    {
      rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s",",");
    }
  }
  if(setDebug & findChipDebug)
  {
    myDebug.println();
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

  if( setDebug & chipStatusLED)
    digitalWrite(LED2, LOW); //start updateStatus sync

  switch(chip[x].chipAddr[0])
  {
    
    case ds2762ID:
    {
      if(millis() >= chip[x].tempTimer + ds2762UpdateTime)
      {
        if(setDebug & ds2762Debug)
        {
          startTime = millis();
          myDebug.println(F("Enter Read DS2762 Lookup"));
        }
        Read_TC_Volts(x);
        Read_CJ_Temp(x);
        cjComp = pgm_read_word_near(kTable + cjTemperature);
        if(setDebug & ds2762Debug)
        {
          myDebug.print(F("kTable["));
          myDebug.print(cjTemperature);
          myDebug.print(F("] = "));
          myDebug.println(pgm_read_word_near(kTable + cjTemperature));
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
          myDebug.print(F("cjComp = "));
          myDebug.print(cjComp);
          myDebug.println(F(" microvolts"));
        }
        tblHi = kTableCnt - 1;
        TC_Lookup();
        if(error == 0)
        {
          if(setDebug & ds2762Debug)
          {
            myDebug.print(F("Temp = "));
            myDebug.print(eePntr);
            myDebug.print(F(" degrees C, "));
            myDebug.print(((eePntr * 9) / 5) + 32);
            myDebug.println(F(" degrees F"));
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
            myDebug.println(F("Value Out Of Range"));
          }
        }
        if(setDebug & ds2762Debug)
        {
          endTime = millis();
          myDebug.print(F("Exit Read DS2762 Lookup - "));
          myDebug.print(endTime - startTime);
          myDebug.println(F(" milliseconds"));
          myDebug.println();
          myDebug.println();
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
            myDebug.print(F("crc Error chip["));
            myDebug.print(x);
            myDebug.println(F("], resetting timer"));
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
          myDebug.print(F("chip "));
          myDebug.print(x);
          myDebug.print(F(" chipCRC = 0X"));
          myDebug.print(chipCRCval, HEX);
          myDebug.print(F(", chipBufferCRC = 0X"));
          myDebug.println(chipBufferCRC, HEX);
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
  if( setDebug & chipStatusLED)
  {  
    digitalWrite(LED2, HIGH); //stop updateStatus sync
  }else{
    delayMicroseconds(100);
  }
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
        myDebug.print(F("tempStrCnt for "));
        myDebug.print(action[x].tempPtr->chipName);
        myDebug.print(F(" is "));
        myDebug.println(tempStrCnt);
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
        myDebug.print(F("tempStrCnt for action["));
        myDebug.print(x);
        myDebug.print(F("]tempPtr.->chipStatus"));
        myDebug.print(F(" is "));
        myDebug.print(tempStrCnt);
        myDebug.print(F(" and the chipStatus is "));
        myDebug.println((int16_t) action[x].tempPtr->chipStatus);
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
          myDebug.print(F("tempStrCnt for "));
          myDebug.print(action[x].tcPtr->chipName);
          myDebug.print(F(" is "));
          myDebug.println(tempStrCnt);
        }else{
          myDebug.print(F("action["));
          myDebug.print(x);
          myDebug.println(F("].tcPtr->chipName is not used"));
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
          myDebug.print(F("tempStrCnt for "));
          myDebug.print(action[x].thPtr->chipName);
          myDebug.print(F(" is "));
          myDebug.println(tempStrCnt);
        }else{
          myDebug.print(F("action["));
          myDebug.print(x);
          myDebug.println(F("].thPtr->chipName is not used"));
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
        myDebug.print(F("lcdupdate took "));
        myDebug.print(lcdUpdateStop - lcdUpdateStart);
        myDebug.println(F(" milliseconds"));
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
    lcdCenterStr(I2CStr, lcdChars);
    lcd[7]->print(lcdStr);
    lcd[7]->setCursor(0, 2);
    lcdCenterStr("Bytes Cleared", lcdChars);
    lcd[7]->print(lcdStr);
  }
  if(setDebug & eepromDebug)
  {
    myDebug.println(F("Exiting readStructures"));
    myDebug.println(F("Chip Structure"));
    displayStructure((byte *)(uint32_t) &chip, sizeof(chip), sizeof(chipStruct));
    myDebug.println(F("Action Structure"));
    displayStructure((byte *)(uint32_t) &action, sizeof(action), sizeof(chipActionStruct));
    myDebug.println(F("PID Structure"));
    displayStructure((byte *)(uint32_t) &ePID, sizeof(ePID), sizeof(chipPIDStruct));
  }
  lcd[7]->setCursor(0, 3);
  lcdCenterStr("Finished", lcdChars);
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
    myDebug.println(F("Enter Read_TC_Volts"));
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
      myDebug.print(F("voltage["));
      myDebug.print(i);
      myDebug.print(F("] = 0x"));
      if(voltage[i] < 0x10){myDebug.print(F("0"));}
      myDebug.print(voltage[i], HEX);
      myDebug.print(F(" "));
    }
  }
  if(setDebug & ds2762Debug)
  {
    myDebug.println();
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
    myDebug.print(F("tcVoltage = "));
    myDebug.print(tcVoltage);
    myDebug.println(F(" microvolts"));
    myDebug.println(F("Exit Read_TC_Volts"));
  }
} 

/* Reads cold junction (device) temperature 
-- each raw bit = 0.125 degrees C 
-- returns tmpCJ in whole degrees C */ 
void Read_CJ_Temp(uint8_t x)
{ 
  if(setDebug & ds2762Debug)
  {
    myDebug.println(F("Enter Read_CJ_Temp"));
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
      myDebug.print(F("cjTemp["));
      myDebug.print(i);
      myDebug.print(F("] = 0x"));
      if(cjTemp[i] < 0x10){myDebug.print(F("0"));}
      myDebug.print(cjTemp[i], HEX);
      myDebug.print(F(" "));
    }
  }
  if(setDebug & ds2762Debug)
  {
    myDebug.println();
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
    myDebug.print(F("cjTemperature = "));
    myDebug.print(cjTemperature);
    myDebug.print(F(" degrees C, "));
    myDebug.print(((cjTemperature * 9) / 5) + 32);
    myDebug.println(F(" degrees F")); 
    myDebug.println(F("Exit Read_CJ_Temp"));
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
    myDebug.println(F("Enter TC_Lookup"));
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
        myDebug.print(F("testVal = "));
        myDebug.print(testVal);
      }
      if(cjComp == testVal)
      {
        if(setDebug & ds2762Debug)
        {
          myDebug.println(F(" - TC_Lookup Temp Match"));
        }
//        tempC = eePntr;
        return; // found it! 
      }else{
        if(cjComp<testVal)
        {
          if(setDebug & ds2762Debug)
          {
             myDebug.println(F(" - testVal too BIG"));
          }
         tblHi=eePntr; //search lower half
        }else{
          if(setDebug & ds2762Debug)
          {
             myDebug.println(F(" - testVal too small"));
          }
         tblLo=eePntr; // search upper half
        }
      }
      if(setDebug & ds2762Debug)
      {
        myDebug.print(F("tblHi = "));
        myDebug.print(tblHi);
        myDebug.print(F(", tblLo = "));
        myDebug.println(tblLo);
      }
      if((tblHi-tblLo)<2)
      { // span at minimum 
        if(setDebug & ds2762Debug)
        {
          myDebug.println(F("TC_Lookup Temp Span At Minimum"));
        }
        eePntr=tblLo; 
        return; 
      } 
    } 
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
      myDebug.println(F("String was too long for the LCD display"));
    }
  }else if(strlen(str) == len){
    strcpy(lcdStr, str);
    if(setDebug & lcdDebug)
    {
      myDebug.println(F("String was exactly right for the LCD display"));
    }
  }else{
    
    if(setDebug & lcdDebug)
    {
      myDebug.print(F("Input String = "));
      myDebug.println(str);
      myDebug.print(F("strlen(str) = "));
      myDebug.print(strlen(str));
      myDebug.println(F(" bytes"));
    }

    lcdPad = (len - strlen(str)) / 2;
    if(setDebug & lcdDebug)
    {
      myDebug.print(F("lcdPad = "));
      myDebug.println(lcdPad);
    }

    memcpy(&lcdStr[lcdPad], str, strlen(str));
    if(setDebug & lcdDebug)
    {
      myDebug.println(F("String was smaller than the LCD Display"));
      myDebug.print(F("lcdStr = \""));
      myDebug.print(lcdStr);
      myDebug.println(F("\""));
    }
  }
}

void checkMasterStop(void)
{
  if(setDebug & resetDebug)
  { 
    myDebug.println(F("Checking Master Stop"));
  }
  elapsedMillis MSTimer = 0;
  while(digitalRead(hwMasterStopPin) == LOW)
  {
    if(MSTimer >= 1000)
    {
      if(setDebug & resetDebug)
      { 
        myDebug.println(F("Executing Master Stop"));
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
    myDebug.println(F("Checking Reset"));
  }
  elapsedMillis resetTimer = 0;
  while(digitalRead(chipResetPin) == LOW)
  {
    if(resetTimer >= 1000)
    {
      if(setDebug & resetDebug)
      { 
        myDebug.println(F("Executing Reset"));
      }
      MasterStop();
      softReset();
      break;
    }
  }
}

void KickDog(void)
{
  digitalWrite(LED5, LOW);
  if(setDebug & wdDebug)
  {
    myDebug.println("Kicking the dog!");
  }
  noInterrupts();
  WDOG_REFRESH = 0xA602;
  WDOG_REFRESH = 0xB480;
  interrupts();
  digitalWrite(LED5, HIGH);
}


/**** Begin TeensyNetGLCD functions ****/
void sendCommand(uint8_t deviceType, uint8_t device, uint32_t  flags ,
                 uint8_t font, uint8_t bGR, uint8_t cR,
                 uint8_t dispP, uint8_t lineP, uint16_t chrP, 
                 char *dSTR, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t rad, uint8_t bRDY)
{

  uint8_t len, x, i;

  if(setDebug & glcdDebug)
  {
    digitalWrite(LED5, LOW);
  }
  glcdUNION.glcdBUF.flags = flags;
  glcdUNION.glcdBUF.font = font;
  glcdUNION.glcdBUF.bGR =  bGR;
  glcdUNION.glcdBUF.cR = cR;
  glcdUNION.glcdBUF.dispP = dispP;
  glcdUNION.glcdBUF.lineP = lineP;
  glcdUNION.glcdBUF.chrP = chrP;
  sprintf(glcdUNION.glcdBUF.dSTR, "%s", dSTR);
  glcdUNION.glcdBUF.x1 = x1;
  glcdUNION.glcdBUF.y1 = y1;
  glcdUNION.glcdBUF.x2 = x2;
  glcdUNION.glcdBUF.y2 = y2;
  glcdUNION.glcdBUF.rad = rad;
  glcdUNION.glcdBUF.bRDY = 1;

  x=0;
  if(deviceType == dsDevice)
  {
    if(setDebug & glcdSerialDebug)
    {
      myDebug.print(F("GLCD Addr: "));
      myDebug.print(x);
      myDebug.print(F(" = "));
      for( i = 0; i < 8; i++)
      {
        myDebug.print(F("0x"));
        if(glcd1w[device].Addr[i] < 16) myDebug.print(F("0"));
        myDebug.print(glcd1w[device].Addr[i], HEX);
        myDebug.print(F(" "));
      }
      myDebug.println();
    }else{
      delayMicroseconds(50);
    }
    if( (setDebug & glcdSerialDebug) || (setDebug & glcdSerialDebug) )
    {
      digitalWrite(LED4, LOW);
    }
    ds.reset();
    ds.select(glcd1w[device].Addr);
    ds.write(0x0F);                     // Write to GLCD
    ds.write_bytes(glcdUNION.glcdARRAY, sizeof(glcdCMD));
    if( (setDebug & glcdSerialDebug) || (setDebug & glcdSerialDebug) )
    {
      digitalWrite(LED4, HIGH);
    }
    if(setDebug & glcdSerialDebug)
    {
      myDebug.print(F("DATA = "));
      for( i = 0; i < sizeof(glcdCMD); i++)
      {
        myDebug.print(F("0x"));
        if(glcdUNION.glcdARRAY[i] < 16) myDebug.print(F("0"));
        myDebug.print(glcdUNION.glcdARRAY[i], HEX);
        myDebug.print(F(" "));
      }
      myDebug.println();
    }
    if(setDebug & glcdSerialDebug)
    {
      delay(50);
    }else{
      delay(40);
    }
  }else
  {
  }
  if(setDebug & glcdDebug)
  {
    digitalWrite(LED5, HIGH);
  }
}

void clearCommandBuf(void)
{
  for(uint8_t x = 0; x< sizeof(glcdCMD); x++)
  {
    glcdUNION.glcdARRAY[x] = 0;
  }
}

void clearDisplayStr(void)
{
  uint8_t x;
  
  for(x = 0; x < displayStrSize; x++)
  {
    displayStr[x] = ' ';
  }
  displayStr[x] = 0x00;
}
/**** End   TeensyNetGLCD functions ****/
void sendUDPpacket(void)
{
  int v = 0, q = 0;
  
  if(setDebug & udpDebug)
  {
    myDebug.print(F("rBuffCnt = "));
    myDebug.println(rBuffCnt);
    myDebug.println(F("ReplyBuffer:"));
    for(q = 0; q < rBuffCnt; q++)
    {
      if(ReplyBuffer[q] != 0x00)
      {
        myDebug.write(ReplyBuffer[q]);
        if(ReplyBuffer[q] == ';')
        {
          myDebug.println();
        }
      }
    }
    myDebug.println();
    if(setDebug & udpHexBuff)
    {
      for(q = 0; q < rBuffCnt; q++)
      {
        myDebug.print(F("0x"));
        if(ReplyBuffer[q] < 0x10)
        {
          myDebug.print(F("0"));
        }
        myDebug.print(ReplyBuffer[q], HEX);
        if(v <= 14)
        {
          myDebug.print(F(" "));
          v++;
        }else{
          myDebug.println();
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

  if(setDebug & udpProcessLED)
    digitalWrite(LED3, LOW); // start updProcess sync
    
  if(setDebug & ethDebug) // display Process Command Letter
  {
    lcd[7]->setCursor(0, 3);
    lcd[7]->print(F("udpProcess - "));
    lcd[7]->print(PacketBuffer[0]);
    lcd[7]->print(F("      "));
  }

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
        myDebug.print(F("Bounjour Name set to: "));
        myDebug.println(bonjourNameBuf);
      }
      EthernetBonjour.removeAllServiceRecords();
      if(setDebug & bonjourDebug)
      {
        myDebug.println(F("Bounjour Service Records Removed"));
        myDebug.print(F("Setting Bonjour Service record to "));
      }
      sprintf(bonjourBuf, "%s._discover", bonjourNameBuf);
      if(setDebug & bonjourDebug)
      {
        myDebug.println(bonjourBuf);
      }
      EthernetBonjour.addServiceRecord(bonjourBuf, localPort, MDNSServiceUDP);
      
      I2CEEPROM_writeAnything(I2CEEPROMbjAddr, bonjourNameBuf, I2C0x50);
      if(setDebug & bonjourDebug)
      {
        myDebug.print(F("Saving "));
        myDebug.print(bonjourNameBuf);
        myDebug.println(F(" to I2CEEPROM"));
      }
      lcd[7]->setCursor(0, 0);
      lcdCenterStr(bonjourNameBuf, lcdChars);
      lcd[7]->print(lcdStr);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "Service Record set to %s", bonjourBuf);
      sendUDPpacket();
      break;
    }
    

    case getActionArray: // "A"
    {
      if(setDebug & udpDebug)
      {
        myDebug.println(F("getActionArray"));
      }
      x = atoi((char *) &PacketBuffer[1]);
      if(x >= maxActions)
      {
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "ERROR");
      }else{
        if(setDebug & udpDebug)
        {
          myDebug.print(F("x = "));
          myDebug.println(x);
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
        myDebug.println(PacketBuffer);
      }
     
      result = strtok( PacketBuffer, delim );

      while(1)
      {
        if(setDebug & actionDebug)
        {
          myDebug.print(F("resultCnt = "));
          myDebug.println(resultCnt);
        }
        
        result = strtok( NULL, delim );
        
        if(setDebug & actionDebug)
        {
          myDebug.print(F("result = "));
          myDebug.println(result);
        }
        
        if(result == NULL){break;}
        
        switch (resultCnt)
        {
          case 0: // action
          {
            actionArray = atoi(result);
            if(setDebug & actionDebug)
            {
              myDebug.print(F("Case 0: actionArray = "));
              myDebug.println(actionArray);
            }
            break;
          }
          case 1:
          {
            actionSection = atoi(result);
            if(setDebug & actionDebug)
            {
              myDebug.print(F("Case 1: actionSection = "));
              myDebug.println(actionSection);
            }
            break;
          }
          
          case 2:
          {
            actionEnableTemp = atoi(result);
            
            if(setDebug & actionDebug)
            {
              myDebug.print(F("Case 2: actionEnable = "));
              myDebug.println(actionEnableTemp);
              myDebug.print(F("action["));
              myDebug.print(actionArray);
              myDebug.print(F("]"));
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
                    myDebug.println(F(".actionEnabled is Enabled"));
                  }
                  
                }else{
                  action[actionArray].actionEnabled = FALSE;
                  
                  if(setDebug & actionDebug)
                  {
                    myDebug.println(F(".actionEnabled is Disabled"));
                  }
                }
                
              if(setDebug & actionDebug)
                {
                  myDebug.print(F("action["));
                  myDebug.print(actionArray);
                  myDebug.print(F("].actionEnabled = "));
                  myDebug.println(action[actionArray].actionEnabled);
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
                    myDebug.print(F(".tooCold is set to "));
                    myDebug.println(actionEnableTemp);
                  }
                  
                }else if( actionSection == 3){
                  action[actionArray].tooHot = actionEnableTemp;
                  
                  if(setDebug & actionDebug)
                  {
                    myDebug.print(F(".tooHot is set to "));
                    myDebug.println(actionEnableTemp);
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
                myDebug.print(F("Case 3: result = "));
                myDebug.println(result);
              }
              actionDelayVal = ((uint32_t) atoi(result));
              
              if(setDebug & actionDebug)
              {
                myDebug.print(F("actionDelayVal = "));
                myDebug.println(actionDelayVal);
              }
              
              actionDelayVal *= 1000;
              
              if(setDebug & actionDebug)
              {
                myDebug.print(F("actionDelayVal * 1000 = "));
                myDebug.println(actionDelayVal);
                myDebug.print(F("action["));
                myDebug.print(actionArray);
                myDebug.print(F("]."));
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
                  myDebug.print(F("tcDelay = "));
                  myDebug.println((actionDelayVal / 1000));
                }
                
              }else if (actionSection == 3){
                action[actionArray].thDelay = actionDelayVal;
                if(actionDelayVal > 0)
                {
                  action[actionArray].thMillis = millis();
                }
                
                if(setDebug & actionDebug)
                {
                  myDebug.print(F("thDelay = "));
                  myDebug.println(actionDelayVal / 1000);
                }
              }
            }
            break;
          }
          
          case 5:
          {
            if(setDebug & actionDebug)
            {
              myDebug.print(F("Case 5 addrResult = "));
              myDebug.println(result);
            }
            addrResult = strtok( result, addrDelim );
            while(addrResult != NULL)
            {
              addrVal[addrResultCnt] = (uint8_t) strtol(addrResult, NULL, 16);
              
              if(setDebug & actionDebug)
              {
                myDebug.print(F(" "));
                myDebug.print(addrVal[addrResultCnt], HEX);
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
              myDebug.println();
              myDebug.print(F("chipAddrCnt =  "));
              myDebug.println(chipAddrCnt, HEX);
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
                myDebug.print(F("Case 4 LCD = "));
                myDebug.println(result);
              }
              action[actionArray].lcdAddr = atoi(result);
              if(setDebug & actionDebug)
              {
                myDebug.print(F("action["));
                myDebug.print(actionArray);
                myDebug.print(F("].lcdAddr = "));
                myDebug.println(action[actionArray].lcdAddr);
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
        myDebug.println(F("masterPidStop Enter"));
      }
      
      for(x=0;x<maxPIDs;x++)
      {
        ePID[x].pidEnabled = FALSE;
        
        if(setDebug & pidDebug)
        {
          myDebug.print(F("ePID["));
          myDebug.print(x);
          myDebug.println(F("].pidEnabled set to FALSE"));
        }
        
        ePID[x].myPID->SetMode(MANUAL);
        
        if(setDebug & pidDebug)
        {
          myDebug.print(F("ePID["));
          myDebug.print(x);
          myDebug.println(F("].myPID->SetMode() set to MANUAL"));
        }
        
        if(&ePID[x].switchPtr->chipAddr != NULL)
        {
          actionSwitchSet((uint8_t *) &ePID[x].switchPtr->chipAddr, ds2406PIOAoff);
          if(setDebug & pidDebug)
          {
              myDebug.print(F("ePID["));
              myDebug.print(x);
              myDebug.println(F("].switchPtr->chipAddr set to OFF"));
          }
        }
      }
      
      if(setDebug & pidDebug)
      {
        myDebug.println(F("masterPidStop Exit"));
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
        myDebug.println(F("updatePidArray Enter"));
        myDebug.println(PacketBuffer);
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
        myDebug.print(F("pidArray = "));
        myDebug.println(pidArray);
        myDebug.print(F("pidEnabledPtr = "));
        myDebug.println(pidEnabledPtr);
        myDebug.print(F("pidTempAddrPtr = "));
        myDebug.println(pidTempAddrPtr);
        myDebug.print(F("pidSetPointPtr = "));
        myDebug.println(pidSetPointPtr);
        myDebug.print(F("pidSwitchAddrPtr = "));
        myDebug.println(pidSwitchAddrPtr);
        myDebug.print(F("pidKpPtr = "));
        myDebug.println(pidKpPtr);
        myDebug.print(F("pidKiPtr = "));
        myDebug.println(pidKiPtr);
        myDebug.print(F("pidKdPtr = "));
        myDebug.println(pidKdPtr);
        myDebug.print(F("pidDirectionPtr = "));
        myDebug.println(pidDirectionPtr);
        myDebug.print(F("pidWindowSizePtr = "));
        myDebug.println(pidWindowSizePtr);
      }
  
      if(setDebug & pidDebug)
      {
        myDebug.print(F("pidEnabled = "));
        myDebug.println(ePID[pidArray].pidEnabled);
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
        myDebug.print(F("chipAddrCnt = "));
        myDebug.println(chipAddrCnt);
      }
      
      if(chipAddrCnt > chipCnt)
      {
        ePID[pidArray].tempPtr = NULL;
      }else{
        ePID[pidArray].tempPtr = &chip[chipAddrCnt];
      }
      
      if(setDebug & pidDebug)
      {
        myDebug.print(F("tempPtr = "));
        myDebug.println((uint32_t) ePID[pidArray].tempPtr, HEX);
      }

      ePID[pidArray].pidSetPoint = strtod(pidSetPointPtr, NULL);
      
      if(setDebug & pidDebug)
      {
        myDebug.print(F("pidSetPoint = "));
        myDebug.println((double) ePID[pidArray].pidSetPoint);
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
        myDebug.print(F("chipAddrCnt = "));
        myDebug.println(chipAddrCnt);
      }
      
      if(chipAddrCnt > chipCnt)
      {
        ePID[pidArray].switchPtr = NULL;
      }else{
        ePID[pidArray].switchPtr = &chip[chipAddrCnt];
      }
      
      if(setDebug & pidDebug)
      {
        myDebug.print(F("switchPtr = "));
        myDebug.println((uint32_t) ePID[pidArray].switchPtr, HEX);
      }

      ePID[pidArray].pidKp = strtod(pidKpPtr, NULL);
      
      if(setDebug & pidDebug)
      {
        myDebug.print(F("pidKp = "));
        myDebug.println((double) ePID[pidArray].pidKp);
      }

      ePID[pidArray].pidKi = strtod(pidKiPtr, NULL);
      
      if(setDebug & pidDebug)
      {
        myDebug.print(F("pidKi = "));
        myDebug.println((double) ePID[pidArray].pidKi);
      }

      ePID[pidArray].pidKd = strtod(pidKdPtr, NULL);
      
      if(setDebug & pidDebug)
      {
        myDebug.print(F("pidKd = "));
        myDebug.println((double) ePID[pidArray].pidKd);
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
        myDebug.print(F("pidDirection = "));
        myDebug.println(ePID[pidArray].pidDirection);
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
        myDebug.print(F("pidEnabled = "));
        myDebug.println(ePID[pidArray].pidEnabled);
      }
      
      if(setDebug & pidDebug)
      {
        myDebug.print(F("pidWindowSize = "));
        myDebug.println((double) ePID[pidArray].pidWindowSize);
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
        myDebug.println(F("updateChipName Enter"));
        myDebug.println(PacketBuffer);
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
        myDebug.print(F("chip["));
        myDebug.print(chipAddrCnt);
        myDebug.print(F("].chipName = "));
        myDebug.println(chip[chipAddrCnt].chipName);
        for(cnCnt = 0; cnCnt < chipNameSize; cnCnt++)
        {
          myDebug.print(F("0x"));
          if(chip[chipAddrCnt].chipName[cnCnt] < 0x0f)
          {
            myDebug.print(F("0"));
          }
          myDebug.print(chip[chipAddrCnt].chipName[cnCnt], HEX);
          if(cnCnt < chipNameSize - 1)
          {
            myDebug.print(F(", "));
          }
        }
        myDebug.println();
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
      if(PacketBuffer[1] == ' ')
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
      }else{
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "INVALID");
      }
      sendUDPpacket();
      break;
    }

    case getMaxGLCDs: // "V"
    {
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d", maxGLCDs);
      sendUDPpacket();
      break;
    }
    
    case getGLCDcnt: // "W"
    {
      I2CEEPROM_readAnything(I2CEEPROMglAddr, numGLCDs, I2C0x50);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d", numGLCDs);
      sendUDPpacket();
      break;
    }
    
    case getGLCDstatus: // "X"
    {
      x = atoi((char *) &PacketBuffer[1]);
     if(x >= maxGLCDs)
      {
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "ERROR");
      }else{
        myDebug.print(F("glcd1w["));
        myDebug.print(x);
        myDebug.print(F("].Name = "));
        myDebug.println(glcd1w[x].Name);
        if( (glcd1w[x].Name[0] == ' ') || (glcd1w[x].Name[0] == 0x00) )
        {
          rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "NULL,",x);
        }else{
          rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s,", glcd1w[x].Name);
        }
        rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%02x,",glcd1w[x].Flags);
        for( uint8_t ActionCnt = 0; ActionCnt < maxGLCDactions; ActionCnt++)
        {
          if(glcd1w[x].Action[ActionCnt] == NULL)
          {
            rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s,","NULL");
          }else{
            rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%08x,",glcd1w[x].Action[ActionCnt]);
          }
        }
        for( uint8_t ChipCnt = 0; ChipCnt < maxGLCDchips; ChipCnt++)
        {
          if(glcd1w[x].Chip[ChipCnt] == NULL)
          {
            rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s,","NULL");
          }else{
            rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%08x,",glcd1w[x].Chip[ChipCnt]);
          }
        }
        for(uint8_t addrCnt = 0; addrCnt < chipAddrSize; addrCnt++)
        {
          rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%02x;",glcd1w[x].Addr[addrCnt]);
        }
      }
      ReplyBuffer[rBuffCnt-1] = 0x00;  // remove the last "," and terminate the string
      sendUDPpacket();
      break;
    }

    case setGLCD: // "Y" Y DEV FLAGS POSITION ACTION
    {
      if(PacketBuffer[1] == ' ')
      {
        result = strtok( PacketBuffer, delim );
        char* dev       = strtok( NULL, delim );
        char* flags     = strtok( NULL, delim );
        char* position  = strtok( NULL, delim );
        char* actionNum = strtok( NULL, delim );
      
        x = atoi(dev);
        glcd1w[x].Flags = atoi(flags);
        uint8_t glcdPosition = atoi(position);
        uint32_t glcdAction = strtoul(actionNum, NULL, 16);
        glcd1w[x].Action[glcdPosition] = (chipActionStruct *) glcdAction;
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d,%d,%d %08x", x, glcd1w[x].Flags, glcdPosition, glcd1w[x].Action[glcdPosition]);
      }else{
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "INVALID");
      }
      sendUDPpacket();
      break;
    }

    case getStructAddr: // "Z"
    {
      x = atoi((char *) &PacketBuffer[2]);
      switch(PacketBuffer[1])
      {
        case 'A': //Actions
        {
          if(x > maxActions)
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "INVALID - Action Too Large");
          }else{
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%08x", &action[x]);
          }
          break;
        }
        
        case 'C': //chips
        {
          if(x > maxChips)
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "INVALID - Chip Too Large");
          }else{
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%08x", &chip[x]);
          }
          break;
        }
        
        case 'P': //PIDs
        {
          if(x > maxPIDs)
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "INVALID - PID Too Large");
          }else{
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%08x", &ePID[x]);
          }
          break;
        }
        
        default:
        {
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "INVALID Selection");
          break;
        }
      }
      sendUDPpacket();
      break;
    }

    case updateglcd1wName: // "a"
    {
      uint8_t cnCnt;
      
      if(setDebug & glcdNameUpdate)
      {
        myDebug.println(F("updateglcd1wName Enter"));
        myDebug.println(PacketBuffer);
      }
      
      result = strtok( PacketBuffer, delim );
      char* glcd1wNameAddr      = strtok( NULL, delim );
      char* glcd1wNameStr       = strtok( NULL, delim ); 
     
      if(setDebug & glcdNameUpdate)
      {
        myDebug.print(F("glcd1wNameAddr = "));
        for(cnCnt = 0; cnCnt < chipNameSize; cnCnt++)
        {
          myDebug.print(F("0x"));
          if(glcd1wNameAddr[cnCnt] < 0x0f)
          {
            myDebug.print(F("0"));
          }
          myDebug.print(glcd1wNameAddr[cnCnt], HEX);
          if(cnCnt < chipNameSize - 1)
          {
            myDebug.print(F(","));
          }
        }
        myDebug.println();
        myDebug.print(F("glcd1wNameStr = "));
        myDebug.println(glcd1wNameStr);
      }
      
      asciiArrayToHexArray(glcd1wNameAddr, addrDelim, addrVal);
      uint8_t glcd1wAddrCnt = matchglcd1wAddress(addrVal);
      
      if(setDebug & glcdNameUpdate)
      {
        myDebug.print(F("glcd1wAddrCnt = "));
        myDebug.println(glcd1wAddrCnt);
      }
      for(cnCnt = 0; cnCnt < chipNameSize; cnCnt++)  // clear the name array
      {
        glcd1w[glcd1wAddrCnt].Name[cnCnt] = 0x00;
      }
      
      strcpy(glcd1w[glcd1wAddrCnt].Name, glcd1wNameStr); //copy the name array

      for(cnCnt = 0; cnCnt < chipNameSize; cnCnt++)
      {
        if(glcd1w[glcd1wAddrCnt].Name[cnCnt] < 0x30 || glcd1w[glcd1wAddrCnt].Name[cnCnt] > 0x7A)
        {
          glcd1w[glcd1wAddrCnt].Name[cnCnt] = 0x00; //remove non-ascii characters
        }
      }

      if(setDebug & glcdNameUpdate)
      {
        myDebug.print(F("glcd1w["));
        myDebug.print(glcd1wAddrCnt);
        myDebug.print(F("].Name = "));
        myDebug.println(glcd1w[glcd1wAddrCnt].Name);
        for(cnCnt = 0; cnCnt < chipNameSize; cnCnt++)
        {
          myDebug.print(F("0x"));
          if(glcd1w[glcd1wAddrCnt].Name[cnCnt] < 0x0f)
          {
            myDebug.print(F("0"));
          }
          myDebug.print(glcd1w[glcd1wAddrCnt].Name[cnCnt], HEX);
          if(cnCnt < chipNameSize - 1)
          {
            myDebug.print(F(", "));
          }
        }
        myDebug.println();
      }
      
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","Name Updated");
      sendUDPpacket();
      break;
    }
    

  case resetGLCD: // "b"
    {
      x = atoi((char *) &PacketBuffer[1]);
      resetGLCDdisplay(x);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s%d Reset","GLCD", x);
      sendUDPpacket();
      break;
    }

  case setDebugPort: // "c"
    {
      x = atoi((char *) &PacketBuffer[1]);
      if(x == 2)
      {
        Stream &myDebug = Serial2;
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "myDebug set to Serial2");
      }else{
        Stream &myDebug = Serial;
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "myDebug set to Serial");
      }
      sendUDPpacket();
      break;
    }

  case resetTeensy: // "r"
    {
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","Resetting TeensyNet");
      sendUDPpacket();
      softReset();
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
        myDebug.print(F("lcdMessage is "));
        myDebug.println(lcdMessage);
        for( int t = 0; t < msgLength; t++)
        {
          myDebug.print(F("0x"));
          if(lcdMessage[t] < 0x10)
          {
            myDebug.print(F("0"));
          }
          myDebug.print(lcdMessage[t], HEX);
          if(t < (msgLength - 1))
          {
            myDebug.print(F(", "));
          }
        }
        myDebug.println();
      }
      
      if( msgLength > 21 )
      {
        if(setDebug & lcdDebug)
        {
          myDebug.print(F("lcdMessage is "));
          myDebug.print(msgLength);
          myDebug.println(F(" too long, truncating"));
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
        myDebug.println(F("MasterStop() completed"));
        delay(1000);
      }
      
      digitalWrite(LED1, LOW); // indicate that I2CEEPROM is being accessed
      EEPROMclear();

      if(setDebug & resetDebug)
      {
        myDebug.println(F("EEPROMclear() completed"));
      }
      I2CEEPROM_readAnything(I2CEEPROMidAddr, i2cEeResult, I2C0x50);
  
      if(setDebug & resetDebug)
      { 
        myDebug.print(F("i2cEeResult = 0x"));
        myDebug.println(i2cEeResult, HEX);
      }
  
      if(i2cEeResult != 0x55)
      {
        if(setDebug & resetDebug)
        { 
          myDebug.println(F("No EEPROM Data"));
          delay(1000);
        }
      }
        
      for(x = 0; x < maxChips; x++)
      {
        memcpy(&chip[x], &chipClear, sizeof(chipStruct));
      }
      
      if(setDebug & resetDebug)
      { 
        myDebug.println(F("chip structures cleared"));
        delay(1000);
      }
  
      for(x = 0; x < maxActions; x++)
      {
        memcpy(&action[x], &actionClear, sizeof(chipActionStruct));
      }
      
      if(setDebug & resetDebug)
      { 
        myDebug.println(F("action structures cleared"));
        delay(1000);
      }
  
      for(x = 0; x < maxPIDs; x++)
      {
        memcpy(&ePID[x], &pidClear, sizeof(chipPIDStruct));
      }
      
      if(setDebug & resetDebug)
      { 
        myDebug.println(F("pid structures cleared"));
        delay(1000);
      }
  
      digitalWrite(LED1, HIGH); // indicate that I2CEEPROM is being accessed

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
  if(setDebug & udpProcessLED)
    digitalWrite(LED3, HIGH); // stop updProcess sync
  KickDog(); // reset Hardware Watchdog
}

void resetGLCDdisplay(uint8_t q)
{
  if(glcd1w[q].Addr[0] == 0x45)
  {
    glcd1w[q].Item     = 0; // reset glcd display items
    glcd1w[q].Position = 0;
    glcd1w[q].Page     = 0;
    if(setDebug & glcdSerialDebug)
    {
      myDebug.print(F("Resetting GLCD "));
      myDebug.print(q);
      myDebug.println(F(" TeensyNet"));
    }
    sendCommand(dsDevice, q, setResetDisplay, 0,0,0,0,0,0,"",0,0,0,0,0,1); //reset the GLCD Display
    delay(2000);
    sendCommand(dsDevice, q, setInitL, 0,0,0,0,0,0,"",0,0,0,0,0,1); //initialize to landscape mode
    for(uint8_t x = 0; x < 8; x++)
    {
      sendCommand(dsDevice, q, setPageWrite, 0,0,0,x,0,0,"",0,0,0,0,0,1); // select page to write to       
      delay(100);
      sendCommand(dsDevice, q, clrScn, 0,0,0,0,0,0,"",0,0,0,0,0,1); // clear the page
      delay(100);
      sendCommand(dsDevice, q, setPageDisplay, 0,0,0,x,0,0,"",0,0,0,0,0,1); // display the page
      delay(100);
      if(setDebug & glcdSerialDebug)
      {
        myDebug.print(F("Initializing GLCD "));
        myDebug.print(q);
        myDebug.print(F(", Page "));
        myDebug.println(x);
      }
      sprintf(displayStr, "GLCD %d, Page %d Initialized", q, x);
      sendCommand(dsDevice, q, (setPrintStr | setFont | setColorVGA), UBUNTUBOLD,0,WHITE,0,0,0,displayStr,0,0,0,0,0,1); // clear the page
      delay(1000);
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
