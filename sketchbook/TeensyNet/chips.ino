/********************

TeensyNet chip functions

Version 0.0.49
Last Modified 04/04/2015
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

void findChips(void)
{
 int cntx = 0, cmpCnt, cmpArrayCnt, dupArray = 0, cnty;

  setLED(LED1, ledON); // start DSO sync
  numberOfGLCDs = 0;
  numberOf1wLCDs = 0;
  ds.reset_search();
  delay(250);
  
  for(cnty = 0; cnty < maxChips; cnty++) // clear all chipnames
  {
    strcpy(chip[cnty].chipName, nullStr);
  }
  
  while (ds.search(chip[cntx].chipAddr))
  {
    if((chip[cntx].chipAddr[0] == dsGLCD) || (chip[cntx].chipAddr[0] == dsGLCDP))
    {
      if(ds.crc8(chip[cntx].chipAddr, chipAddrSize-1) != chip[cntx].chipAddr[chipAddrSize-1])
      {
        continue;
      }
      updateGLCD1W(numberOfGLCDs, cntx);
      numberOfGLCDs++;
      continue;
    }
    
    if(chip[cntx].chipAddr[0] == dsLCD)
    {
      if(ds.crc8(chip[cntx].chipAddr, chipAddrSize-1) != chip[cntx].chipAddr[chipAddrSize-1])
      {
        continue;
      }
      updateLCD1W(numberOf1wLCDs, cntx);
      numberOf1wLCDs++;
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
        myDebug[debugPort]->print(F("Chip "));
        myDebug[debugPort]->print(cntx);
        myDebug[debugPort]->println(F(" - Duplicated Array"));
      }
      dupArray = 0;
      continue;
    }
    
    if(ds.crc8(chip[cntx].chipAddr, chipAddrSize-1) != chip[cntx].chipAddr[chipAddrSize-1])
    {
      if(setDebug & findChipDebug)
      {
        myDebug[debugPort]->print(F("CRC Error - Chip "));
        myDebug[debugPort]->print(cntx);
        myDebug[debugPort]->print(F(" = "));
        myDebug[debugPort]->print(chip[cntx].chipAddr[chipAddrSize-1], HEX);
        myDebug[debugPort]->print(F(", CRC should be "));
        myDebug[debugPort]->println(ds.crc8(chip[cntx].chipAddr, chipAddrSize-1));
      }
      continue;
    }

    if(setDebug & findChipDebug)
    {
      myDebug[debugPort]->print(F("Chip "));
      myDebug[debugPort]->print(cntx);
      myDebug[debugPort]->print(F(" Address 0x"));
      myDebug[debugPort]->print((uint32_t) &(chip[cntx].chipAddr), HEX);
      myDebug[debugPort]->print(F(" = {"));
      
      for( int i = 0; i < chipAddrSize; i++)
      {
        if(chip[cntx].chipAddr[i]>=0 && chip[cntx].chipAddr[i]<16)
        {
          myDebug[debugPort]->print(F("0x0"));
        }else{
          myDebug[debugPort]->print(F("0x"));
        }
        myDebug[debugPort]->print(chip[cntx].chipAddr[i], HEX);
        if(i < 7){myDebug[debugPort]->print(F(","));}
      }
      myDebug[debugPort]->println(F("}"));
    }
      
    cntx++;
    delay(50);
  }

  ds.reset_search();
  delay(250);
  chipCnt = cntx;
  glcdCnt = numberOfGLCDs;
  num1wLCDs = numberOf1wLCDs;
  
  if(cntx < maxChips)
  {
    for(;cntx<maxChips;cntx++)
    {
      for(int y=0;y<chipAddrSize;y++) // clear unused chip slots
      {
        chip[cntx].chipAddr[y]=0;
      }
    }
  }
  
  if(glcdCnt < maxGLCDs)
  {
    for(;glcdCnt<maxGLCDs;glcdCnt++)
    {
      for(int y=0;y<chipAddrSize;y++) // clear unused GLCD slots
      {
        glcd1w[glcdCnt].Addr[y]=0;
      }
    }
  }
  
  if(num1wLCDs < maxLCDs)
  {
    for(;num1wLCDs<maxLCDs;num1wLCDs++)
    {
      for(int y=0;y<chipAddrSize;y++) // clear unused LCD slots
      {
        lcd1w[num1wLCDs].Addr[y]=0;
      }
    }
  }
  
  if(setDebug & findChipDebug)
  {
    myDebug[debugPort]->print(chipCnt);
    myDebug[debugPort]->print(F(" Sensor"));
    if(chipCnt == 1)
    {
      myDebug[debugPort]->println(F(" Detected"));
    }else{
      myDebug[debugPort]->println(F("s Detected"));
    }
    
    myDebug[debugPort]->print(glcdCnt);
    myDebug[debugPort]->print(F(" 1-wire GLCD"));
    if(glcdCnt == 1)
    {
      myDebug[debugPort]->println(F(" Detected"));
    }else{
      myDebug[debugPort]->println(F("s Detected"));
    }
    
    myDebug[debugPort]->print(lcdCnt);
    myDebug[debugPort]->print(F(" 1-wire LCD"));
    if(lcdCnt == 1)
    {
      myDebug[debugPort]->println(F(" Detected"));
    }else{
      myDebug[debugPort]->println(F("s Detected"));
    }
  }
  for(cntx = 0; cntx < maxChips; cntx++)
  {
    switch(chip[cntx].chipAddr[0])
    {
      case ds18b20ID:
      {
        if(setDebug & findChipDebug)
        {
          myDebug[debugPort]->print("Writing to chip[");
          myDebug[debugPort]->print(cntx);
          myDebug[debugPort]->println("] scratchpad");
        }
        ds.reset();
        ds.select(chip[cntx].chipAddr);
        ds.write(0x4E); // write to scratchpad;
        ds.write(0x00); // low alarm
        ds.write(0x00); // high alarm
        ds.write(0x1F); // configuration register - 9 bit accuracy (0.5deg C)
        delay(5);
        ds.reset();
        ds.select(chip[cntx].chipAddr);
        ds.write(0x48); // copy scratchpad to EEPROM;
        delay(5);
        if(setDebug & findChipDebug)
        {
          myDebug[debugPort]->println("Finished writing to scratchpad");
        }
        break;
      }
      
      default:
      {
        break;
      }
    }
  }
    setLED(LED1, ledOFF); // end DSO sync
}


uint8_t matchChipAddress(uint8_t* array)
{
   uint8_t addrMatchCnt, chipAddrCnt;
   
  if(setDebug & pidDebug)
  {
   myDebug[debugPort]->println(F("matchChipAddress"));
  }
  
  for(addrMatchCnt = 0, chipAddrCnt = 0; ((addrMatchCnt < chipAddrSize) || (chipAddrCnt > chipCnt)); addrMatchCnt++)
  {
    if(array[addrMatchCnt] != chip[chipAddrCnt].chipAddr[addrMatchCnt])
    {
      addrMatchCnt = 0;
      chipAddrCnt++;
      
      if(setDebug & pidDebug)
      {
        myDebug[debugPort]->println(chipAddrCnt);
      }
  
      continue;
    }
    
    if(setDebug & pidDebug)
    {
      myDebug[debugPort]->print(array[addrMatchCnt], HEX);
      myDebug[debugPort]->print(F(","));
    }
  }
  
  if(chipAddrCnt <= chipCnt)
  {
    if(setDebug & pidDebug)
    {
      myDebug[debugPort]->print(F("MATCH!! - "));
    }
  }else{

    if(setDebug & pidDebug)
    {
      myDebug[debugPort]->print(F("NO MATCH!! - "));
    }

    chipAddrCnt = 0xFF;
  }

  if(setDebug & pidDebug)
  {
    myDebug[debugPort]->println(chipAddrCnt);
  }

  return(chipAddrCnt);
}

void showChipAddress( uint8_t* array)
{
  if(setDebug & findChipDebug)
  {
    myDebug[debugPort]->print(F("Chip Address 0x"));
    myDebug[debugPort]->print((uint32_t) array, HEX);
    myDebug[debugPort]->print(F(" = "));
  }
  
  for( int i = 0; i < chipAddrSize; i++)
  {
    rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s", "0x");    
    rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%02X",(uint8_t) array[i]);
    if(setDebug & findChipDebug)
    {
      myDebug[debugPort]->print(F("0x"));
      if( array[i] < 0x10 ) myDebug[debugPort]->print(F("0")); 
      myDebug[debugPort]->print((uint8_t) array[i], HEX);
      if(i < 7)myDebug[debugPort]->print(F(","));
    }
    if(i < 7)
    {
      rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, "%s",",");
    }
  }
  if(setDebug & findChipDebug)
  {
    myDebug[debugPort]->println();
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
    setLED(LED2, ledON); //start updateStatusSync

  switch(chip[x].chipAddr[0])
  {
    
    case ds2762ID:
    {
      if(chip[x].tempTimer >= ds2762UpdateTime)
      {
        if(setDebug & ds2762Debug)
        {
          startTime = millis();
          myDebug[debugPort]->println(F("Enter Read DS2762 Lookup"));
        }
        Read_TC_Volts(x);
        Read_CJ_Temp(x);
        cjComp = pgm_read_word_near(kTable + cjTemperature);
        if(setDebug & ds2762Debug)
        {
          myDebug[debugPort]->print(F("kTable["));
          myDebug[debugPort]->print(cjTemperature);
          myDebug[debugPort]->print(F("] = "));
          myDebug[debugPort]->println(pgm_read_word_near(kTable + cjTemperature));
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
          myDebug[debugPort]->print(F("cjComp = "));
          myDebug[debugPort]->print(cjComp);
          myDebug[debugPort]->println(F(" microvolts"));
        }
        tblHi = kTableCnt - 1;
        TC_Lookup();
        if(error == 0)
        {
          if(setDebug & ds2762Debug)
          {
            myDebug[debugPort]->print(F("Temp = "));
            myDebug[debugPort]->print(eePntr);
            myDebug[debugPort]->print(F(" degrees C, "));
            myDebug[debugPort]->print(((eePntr * 9) / 5) + 32);
            myDebug[debugPort]->println(F(" degrees F"));
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
            myDebug[debugPort]->println(F("Value Out Of Range"));
          }
        }
        if(setDebug & ds2762Debug)
        {
          endTime = millis();
          myDebug[debugPort]->print(F("Exit Read DS2762 Lookup - "));
          myDebug[debugPort]->print(endTime - startTime);
          myDebug[debugPort]->println(F(" milliseconds"));
          myDebug[debugPort]->println();
          myDebug[debugPort]->println();
        }
        chip[x].tempTimer = 0;
      }
      break;
    }
    
    case ds18b20ID:
    case t3tcID:
    case max31850ID:
    {
      if((chip[x].tempTimer >= tempReadDelay))
      {
        if(setDebug & chipDebug)
        {
          myDebug[debugPort]->print("chip[");
          myDebug[debugPort]->print(x);
          myDebug[debugPort]->print("].tempTimer = ");
          myDebug[debugPort]->println(chip[x].tempTimer);
        }
        ds.reset();
        ds.select(chip[x].chipAddr);    
        ds.write(0xBE);         // Read Scratchpad
  
        for (int i = 0; i < 9; i++) 
        {
          chipBuffer[i] = ds.read();
          if(setDebug & chipDebug)
          {
            myDebug[debugPort]->print(chipBuffer[i], HEX);
            myDebug[debugPort]->print(" ");
          }
        }
        if(setDebug & chipDebug)
        {
          myDebug[debugPort]->println();
        }
        if(ds.crc8(chipBuffer, 8) != chipBuffer[8])
        {
          if(setDebug & crcDebug)
          {
            myDebug[debugPort]->print(F("crc Error chip["));
            myDebug[debugPort]->print(x);
            myDebug[debugPort]->println(F("], resetting timer"));
          }
          chip[x].tempTimer = 0; // restart the chip times
          break; // CRC invalid, try later
        }
        int16_t raw = (chipBuffer[1] << 8) | chipBuffer[0];
        if( showCelsius == TRUE)
        {
          if(chip[x].chipAddr[0] == 0xAA)
          {
            chip[x].chipStatus = raw;
          }else{
            chip[x].chipStatus =  raw >> 4;
          }
        }else{
          if(chip[x].chipAddr[0] == 0xAA)
          {
            chip[x].chipStatus = ((raw * 9) / 5) + 32 ;
          }else{
            chip[x].chipStatus =  (((raw >> 4) * 9) / 5) + 32;
          }
        }
        ds.reset();
        ds.select(chip[x].chipAddr);
        ds.write(0x44);         // start conversion
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

        if(setDebug & crcDebug)
        {
          myDebug[debugPort]->print(F("chip "));
          myDebug[debugPort]->print(x);
          myDebug[debugPort]->print(F(" chipCRC = 0X"));
          myDebug[debugPort]->print(chipCRCval, HEX);
          myDebug[debugPort]->print(F(", chipBufferCRC = 0X"));
          myDebug[debugPort]->println(chipBufferCRC, HEX);
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
    setLED(LED2, ledOFF); //stop updateStatusSync  
  }else{
    delayMicroseconds(100);
  }
}


