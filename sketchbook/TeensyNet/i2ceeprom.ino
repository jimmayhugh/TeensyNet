/********************

TeensyNet i2ceeprom functions

Version 0.0.49
Last Modified 04/04/2015
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

void readStructures(void)
{  
  if(setDebug & eepromDebug)
  {
    myDebug[debugPort]->println(F("Entering readStructures"));
    myDebug[debugPort]->print(F("I2CEEPROMchipAddr = 0x"));
    myDebug[debugPort]->println(I2CEEPROMchipAddr, HEX);
    myDebug[debugPort]->print(F("I2CEEPROMactionAddr = 0x"));
    myDebug[debugPort]->println(I2CEEPROMactionAddr, HEX);
    myDebug[debugPort]->print(F("I2CEEPROMpidAddr = 0x"));
    myDebug[debugPort]->println(I2CEEPROMpidAddr, HEX);
    myDebug[debugPort]->print(F("I2CEEPROMglcdAddr = 0x"));
    myDebug[debugPort]->println(I2CEEPROMglcdAddr, HEX);
    myDebug[debugPort]->print(F("I2CEEPROMlcdAddr = 0x"));
    myDebug[debugPort]->println(I2CEEPROMlcdAddr, HEX);
  }

  I2CEEPROM_readAnything(I2CEEPROMchipCntAddr, chipCnt, I2C0x50); // get chipCnt
  I2CEEPROM_readAnything(I2CEEPROMnumGLCDsAddr, numberOfGLCDs, I2C0x50); //get numberOfGLCDs
  I2CEEPROM_readAnything(I2CEEPROMnum1wLCDsAddr, numberOf1wLCDs, I2C0x50); //get numberOf1wLCDs

  if(setDebug & eepromDebug)
  {
    myDebug[debugPort]->print(F("Reading 1-wire chip counts"));
    myDebug[debugPort]->print(F("chipCnt = "));
    myDebug[debugPort]->println(chipCnt);
    myDebug[debugPort]->print(F("numberOfGLCDs = "));
    myDebug[debugPort]->println(numberOfGLCDs);
    myDebug[debugPort]->print(F("numberOf1wLCDs = "));
    myDebug[debugPort]->println(numberOf1wLCDs);
  } 

  I2CEEPROM_readAnything(I2CEEPROMidAddr, i2cEeResult, I2C0x50);  
  if(setDebug & eepromDebug)
  { 
    myDebug[debugPort]->print(F("i2cEeResult = 0x"));
    myDebug[debugPort]->println(i2cEeResult, HEX);
  }
  
  if(i2cEeResult != 0x55)
  {
    if(setDebug & eepromDebug)
    { 
       myDebug[debugPort]->println(F("No EEPROM Data"));
    }
    i2cEepromReady = FALSE;
  }else{
    if(setDebug & eepromDebug)
    { 
       myDebug[debugPort]->println(F("EEPROM Data Valid"));
    }
    i2cEepromReady = TRUE;
  }
  
  i2cEeResult16 = I2CEEPROM_readAnything(I2CEEPROMchipAddr, chip, I2C0x50);
  if(setDebug & eepromDebug)
  {
    myDebug[debugPort]->print(F("Read "));
    myDebug[debugPort]->print(i2cEeResult16);
    myDebug[debugPort]->print(F(" bytes from chip address Ox"));
    myDebug[debugPort]->println(I2CEEPROMchipAddr, HEX);
  }

  i2cEeResult16 = I2CEEPROM_readAnything(I2CEEPROMactionAddr, action,  I2C0x50);
  if(setDebug & eepromDebug)
  {
    myDebug[debugPort]->print(F("Read "));
    myDebug[debugPort]->print(i2cEeResult16);
    myDebug[debugPort]->print(F(" bytes from action address Ox"));
    myDebug[debugPort]->println(I2CEEPROMactionAddr, HEX);
  }

  i2cEeResult16 = I2CEEPROM_readAnything(I2CEEPROMpidAddr, ePID,  I2C0x50);
  if(setDebug & eepromDebug)
  {
    myDebug[debugPort]->print(F("Read "));
    myDebug[debugPort]->print(i2cEeResult16);
    myDebug[debugPort]->print(F(" bytes from pid address Ox"));
    myDebug[debugPort]->println(I2CEEPROMpidAddr, HEX);
  }

  i2cEeResult16 = I2CEEPROM_readAnything(I2CEEPROMglcdAddr, glcd1w, I2C0x50);
  if(setDebug & eepromDebug)
  {
    myDebug[debugPort]->print(F("Read "));
    myDebug[debugPort]->print(i2cEeResult16);
    myDebug[debugPort]->print(F(" bytes from glcd address Ox"));
    myDebug[debugPort]->println(I2CEEPROMglcdAddr, HEX);
  }

  i2cEeResult16 = I2CEEPROM_readAnything(I2CEEPROMlcdAddr, lcd1w, I2C0x50);
  if(setDebug & eepromDebug)
  {
    myDebug[debugPort]->print(F("Read "));
    myDebug[debugPort]->print(i2cEeResult16);
    myDebug[debugPort]->print(F(" bytes from lcd address Ox"));
    myDebug[debugPort]->println(I2CEEPROMlcdAddr, HEX);
    myDebug[debugPort]->println();
    myDebug[debugPort]->println(F(" Completed"));
    myDebug[debugPort]->println(F("Exiting readStructures"));
    myDebug[debugPort]->println(F("Chip Structure"));
    displayStructure((byte *)(uint32_t) &chip, sizeof(chip), sizeof(chipStruct));
    myDebug[debugPort]->println(F("Action Structure"));
    displayStructure((byte *)(uint32_t) &action, sizeof(action), sizeof(chipActionStruct));
    myDebug[debugPort]->println(F("PID Structure"));
    displayStructure((byte *)(uint32_t) &ePID, sizeof(ePID), sizeof(chipPIDStruct));
    myDebug[debugPort]->println(F("GLCD Structure"));
    displayStructure((byte *)(uint32_t) &glcd1w, sizeof(glcd1w), sizeof(glcd1wStruct));
    myDebug[debugPort]->println(F("LCD Structure"));
    displayStructure((byte *)(uint32_t) &lcd1w, sizeof(lcd1w), sizeof(t3LCDStruct));
  }

  pidSetup();
}


void saveStructures(void)
{  
  if(setDebug & eepromDebug)
  {
    myDebug[debugPort]->println(F("Entering saveStructures"));
    myDebug[debugPort]->print(F("I2CEEPROMchipAddr = 0x"));
    myDebug[debugPort]->println(I2CEEPROMchipAddr, HEX);
    myDebug[debugPort]->print(F("I2CEEPROMactionAddr = 0x"));
    myDebug[debugPort]->println(I2CEEPROMactionAddr, HEX);
    myDebug[debugPort]->print(F("I2CEEPROMpidAddr = 0x"));
    myDebug[debugPort]->println(I2CEEPROMpidAddr, HEX);
    myDebug[debugPort]->print(F("I2CEEPROMglcdAddr = 0x"));
    myDebug[debugPort]->println(I2CEEPROMglcdAddr, HEX);
    myDebug[debugPort]->print(F("I2CEEPROMlcdAddr = 0x"));
    myDebug[debugPort]->println(I2CEEPROMlcdAddr, HEX);
  }

  if(setDebug & eepromDebug)
  {
    myDebug[debugPort]->print(F("writing 1-wire chip counts"));
    myDebug[debugPort]->print(F("chipCnt = "));
    myDebug[debugPort]->println(chipCnt);
    myDebug[debugPort]->print(F("numberOfGLCDs = "));
    myDebug[debugPort]->println(numberOfGLCDs);
    myDebug[debugPort]->print(F("numberOf1wLCDs = "));
    myDebug[debugPort]->println(numberOf1wLCDs);
    myDebug[debugPort]->print(F("I2CEEPROMidVal = 0x"));
    myDebug[debugPort]->println(I2CEEPROMidVal, HEX);
  } 

  
  I2CEEPROM_writeAnything(I2CEEPROMchipCntAddr, chipCnt, I2C0x50); // save chipCnt
  myDebug[debugPort]->print(F("chipCnt = "));
  myDebug[debugPort]->println(chipCnt);
  I2CEEPROM_writeAnything(I2CEEPROMnumGLCDsAddr, numberOfGLCDs, I2C0x50); //save numberOfGLCDs
  myDebug[debugPort]->print(F("numberOfGLCDs = "));
  myDebug[debugPort]->println(numberOfGLCDs);
  I2CEEPROM_writeAnything(I2CEEPROMnum1wLCDsAddr, numberOf1wLCDs, I2C0x50); // save numberOf1wLCDs
  myDebug[debugPort]->print(F("numberOf1wLCDs = "));
  myDebug[debugPort]->println(numberOf1wLCDs);
  I2CEEPROM_writeAnything(I2CEEPROMidAddr, I2CEEPROMidVal, I2C0x50);
  
  if(setDebug & eepromDebug)
  {
    myDebug[debugPort]->print(F("Wrote "));
    myDebug[debugPort]->print(chipCnt);
    myDebug[debugPort]->println(F(" to  I2CEEPROMccAddr"));
    
    myDebug[debugPort]->print(F("Wrote "));
    myDebug[debugPort]->print(numberOfGLCDs);
    myDebug[debugPort]->println(F(" to  I2CEEPROMglAddr"));
    
    myDebug[debugPort]->print(F("Wrote "));
    myDebug[debugPort]->print(numberOf1wLCDs);
    myDebug[debugPort]->println(F(" to  I2CEEPROMlAddr"));    
  }
 
  i2cEeResult16 = I2CEEPROM_writeAnything(I2CEEPROMchipAddr, chip, I2C0x50);
  if(setDebug & eepromDebug)
  {
    myDebug[debugPort]->print(F("Wrote "));
    myDebug[debugPort]->print(i2cEeResult16);
    myDebug[debugPort]->print(F(" bytes to chip address Ox"));
    myDebug[debugPort]->print(I2CEEPROMchipAddr, HEX);
    myDebug[debugPort]->print(F(" from chip address 0x"));
    uint32_t chipStructAddr = (uint32_t) &chip[0];
    myDebug[debugPort]->println(chipStructAddr, HEX);
  }

  i2cEeResult16 = I2CEEPROM_writeAnything(I2CEEPROMglcdAddr, glcd1w, I2C0x50);
  if(setDebug & eepromDebug)
  {
    myDebug[debugPort]->print(F("Wrote "));
    myDebug[debugPort]->print(i2cEeResult16);
    myDebug[debugPort]->print(F(" bytes to glcd address Ox"));
    myDebug[debugPort]->print(I2CEEPROMglcdAddr, HEX);
    myDebug[debugPort]->print(F(" from glcd address 0x"));
    uint32_t glcd1wStructAddr = (uint32_t) &glcd1w[0];
    myDebug[debugPort]->println(glcd1wStructAddr, HEX);
  }

  i2cEeResult16 = I2CEEPROM_writeAnything(I2CEEPROMlcdAddr, lcd1w, I2C0x50);
  if(setDebug & eepromDebug)
  {
    myDebug[debugPort]->print(F("Wrote "));
    myDebug[debugPort]->print(i2cEeResult16);
    myDebug[debugPort]->print(F(" bytes to lcd1w address Ox"));
    myDebug[debugPort]->print(I2CEEPROMlcdAddr, HEX);
    myDebug[debugPort]->print(F(" from lcd1w address 0x"));
    uint32_t lcd1wStructAddr = (uint32_t) &lcd1w[0];
    myDebug[debugPort]->println(lcd1wStructAddr, HEX);
  }

  i2cEeResult16 = I2CEEPROM_writeAnything(I2CEEPROMactionAddr, action, I2C0x50);
  if(setDebug & eepromDebug)
  {
    myDebug[debugPort]->print(F("Wrote "));
    myDebug[debugPort]->print(i2cEeResult16);
    myDebug[debugPort]->print(F(" bytes to action address Ox"));
    myDebug[debugPort]->print(I2CEEPROMactionAddr, HEX);
    myDebug[debugPort]->print(F(" from action address 0x"));
    uint32_t actionStructAddr = (uint32_t) &action[0];
    myDebug[debugPort]->println(actionStructAddr, HEX);
  }


  i2cEeResult16 = I2CEEPROM_writeAnything(I2CEEPROMpidAddr, ePID, I2C0x50);
  
  if(setDebug & eepromDebug)
  {
    myDebug[debugPort]->print(F("Wrote "));
    myDebug[debugPort]->print(i2cEeResult16);
    myDebug[debugPort]->print(F(" bytes to pid address Ox"));
    myDebug[debugPort]->print(I2CEEPROMpidAddr, HEX);
    myDebug[debugPort]->print(F(" from pid address 0x"));
    uint32_t pidStructAddr = (uint32_t) &ePID[0];
    myDebug[debugPort]->println(pidStructAddr, HEX);
    myDebug[debugPort]->println();
    myDebug[debugPort]->println(F("Completed - Displaying chip Structures"));
    myDebug[debugPort]->println(F("Chip Structure"));
    displayStructure((byte *)(uint32_t) &chip, sizeof(chip), sizeof(chipStruct));
    myDebug[debugPort]->println(F("Action Structure"));
    displayStructure((byte *)(uint32_t) &action, sizeof(action), sizeof(chipActionStruct));
    myDebug[debugPort]->println(F("PID Structure"));
    displayStructure((byte *)(uint32_t) &ePID, sizeof(ePID), sizeof(chipPIDStruct));
    myDebug[debugPort]->println(F("GLCD Structure"));
    displayStructure((byte *)(uint32_t) &glcd1w, sizeof(glcd1w), sizeof(glcd1wStruct));
    myDebug[debugPort]->println(F("LCD Structure"));
    displayStructure((byte *)(uint32_t) &lcd1w, sizeof(lcd1w), sizeof(t3LCDStruct));
  }
  
  I2CEEPROM_readAnything(I2CEEPROMidAddr, i2cEeResult, I2C0x50);
  
  if(setDebug & eepromDebug)
  { 
    myDebug[debugPort]->print(F("i2cEeResult = 0x"));
    myDebug[debugPort]->println(i2cEeResult, HEX);
  }
  
  if(i2cEeResult != 0x55)
  {
    if(setDebug & eepromDebug)
    { 
       myDebug[debugPort]->println(F("EEPROM Data Erased"));
    }
    i2cEepromReady = FALSE;
  }else{
    if(setDebug & eepromDebug)
    { 
       myDebug[debugPort]->println(F("EEPROM Data Valid"));
    }
    i2cEepromReady = TRUE;
  }
  
  if(setDebug & eepromDebug)
  {
    myDebug[debugPort]->println(F("Exiting saveStructures"));
  }  
}

void displayStructure(byte *addr, int xSize, int ySize)
{
  int x, y;
  myDebug[debugPort]->print(F("0x"));
  myDebug[debugPort]->print((uint32_t)addr, HEX);
  myDebug[debugPort]->print(F(": ")); 
  for(x = 0, y = 0; x < xSize; x++)
  {
    if(addr[x] >=0 && addr[x] <= 15)
    {
      myDebug[debugPort]->print(F("0x0"));
    }else{
      myDebug[debugPort]->print(F("0x"));
    }
    myDebug[debugPort]->print(addr[x], HEX);
    y++;
    if(y < ySize)
    {
      myDebug[debugPort]->print(F(", "));
    }else{
      y = 0;
      myDebug[debugPort]->println();
      myDebug[debugPort]->print(F("0x"));
      myDebug[debugPort]->print((uint32_t)addr + x + 1, HEX);
      myDebug[debugPort]->print(F(": ")); 
    }
  }
  myDebug[debugPort]->println();
  myDebug[debugPort]->println();
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
    lcdCenterStr((char *)"Bytes Cleared", lcdChars);
    lcd[7]->print(lcdStr);
  }
  if(setDebug & eepromDebug)
  {
    myDebug[debugPort]->println(F("Exiting readStructures"));
    myDebug[debugPort]->println(F("Chip Structure"));
    displayStructure((byte *)(uint32_t) &chip, sizeof(chip), sizeof(chipStruct));
    myDebug[debugPort]->println(F("Action Structure"));
    displayStructure((byte *)(uint32_t) &action, sizeof(action), sizeof(chipActionStruct));
    myDebug[debugPort]->println(F("PID Structure"));
    displayStructure((byte *)(uint32_t) &ePID, sizeof(ePID), sizeof(chipPIDStruct));
  }
  lcd[7]->setCursor(0, 3);
  lcdCenterStr((char *)"Finished", lcdChars);
  lcd[7]->print(lcdStr);
}


