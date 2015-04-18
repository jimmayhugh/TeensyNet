/********************

TeensyNet updprocess

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
    setLED(LED3, ledON);
//    digitalWrite(LED3, LOW); // start updProcess sync
    
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
      I2CEEPROM_readAnything(I2CEEPROMchipCntAddr, chipCnt, I2C0x50);
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
        myDebug[debugPort]->print(F("Bounjour Name set to: "));
        myDebug[debugPort]->println(bonjourNameBuf);
      }
      EthernetBonjour.removeAllServiceRecords();
      if(setDebug & bonjourDebug)
      {
        myDebug[debugPort]->println(F("Bounjour Service Records Removed"));
        myDebug[debugPort]->print(F("Setting Bonjour Service record to "));
      }
      sprintf(bonjourBuf, "%s._discover", bonjourNameBuf);
      if(setDebug & bonjourDebug)
      {
        myDebug[debugPort]->println(bonjourBuf);
      }
      EthernetBonjour.addServiceRecord(bonjourBuf, localPort, MDNSServiceUDP);
      
      I2CEEPROM_writeAnything(I2CEEPROMbjAddr, bonjourNameBuf, I2C0x50);
      if(setDebug & bonjourDebug)
      {
        myDebug[debugPort]->print(F("Saving "));
        myDebug[debugPort]->print(bonjourNameBuf);
        myDebug[debugPort]->println(F(" to I2CEEPROM"));
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
        myDebug[debugPort]->println(F("getActionArray"));
      }
      x = atoi((char *) &PacketBuffer[1]);
      if(x >= maxActions)
      {
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "ERROR");
      }else{
        if(setDebug & udpDebug)
        {
          myDebug[debugPort]->print(F("x = "));
          myDebug[debugPort]->println(x);
        }
  
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d;",action[x].actionEnabled);
        if(action[x].tempPtr == NULL)
        {
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",charChipAddrArray);
        }else{
          showChipAddress((uint8_t *) &action[x].tempPtr->chipAddr);
        }
        if(action[x].tempPtr == NULL)
        {
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, ";%s", unassignedStr);
        }else{
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, ";%s", action[x].tempPtr->chipName);
        }
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, ";%d",action[x].tooCold);
        if(action[x].tcPtr == NULL)
        {
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, ";%s",charChipAddrArray);
        }else{
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",";");
          showChipAddress((uint8_t *) &action[x].tcPtr->chipAddr);
        }
        if(action[x].tcPtr == NULL)
        {
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, ";%s", unassignedStr);
        }else{
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, ";%s", action[x].tcPtr->chipName);
        }
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, ";%c",(char) action[x].tcSwitchLastState);
        itoa((action[x].tcDelay / 1000), itoaBuf, 10); 
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, ";%s", itoaBuf);
        itoa(action[x].tcMillis, itoaBuf, 10); 
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, ";%s", itoaBuf);
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, ";%d",action[x].tooHot);
        if(action[x].thPtr == NULL)
        {
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, ";%s",charChipAddrArray);
        }else{
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s",";");
          showChipAddress((uint8_t *) &action[x].thPtr->chipAddr);
        }
        if(action[x].thPtr == NULL)
        {
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, ";%s", unassignedStr);
        }else{
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, ";%s", action[x].thPtr->chipName);
        }
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, ";%c",(char) action[x].thSwitchLastState);
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, ";%ld",(action[x].thDelay / 1000));
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, ";%ld",action[x].thMillis);
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, ";%d",action[x].lcdAddr);
      }
      sendUDPpacket();
    }
    break;
    
    case updateActionArray: // "B"
    {
      if(setDebug & actionDebug)
      {
        myDebug[debugPort]->println(PacketBuffer);
      }
     
      result = strtok( PacketBuffer, delim );

      while(1)
      {
        if(setDebug & actionDebug)
        {
          myDebug[debugPort]->print(F("resultCnt = "));
          myDebug[debugPort]->println(resultCnt);
        }
        
        result = strtok( NULL, delim );
        
        if(setDebug & actionDebug)
        {
          myDebug[debugPort]->print(F("result = "));
          myDebug[debugPort]->println(result);
        }
        
        if(result == NULL){break;}
        
        switch (resultCnt)
        {
          case 0: // action
          {
            actionArray = atoi(result);
            if(setDebug & actionDebug)
            {
              myDebug[debugPort]->print(F("Case 0: actionArray = "));
              myDebug[debugPort]->println(actionArray);
            }
            break;
          }
          case 1:
          {
            actionSection = atoi(result);
            if(setDebug & actionDebug)
            {
              myDebug[debugPort]->print(F("Case 1: actionSection = "));
              myDebug[debugPort]->println(actionSection);
            }
            break;
          }
          
          case 2:
          {
            actionEnableTemp = atoi(result);
            
            if(setDebug & actionDebug)
            {
              myDebug[debugPort]->print(F("Case 2: actionEnable = "));
              myDebug[debugPort]->println(actionEnableTemp);
              myDebug[debugPort]->print(F("action["));
              myDebug[debugPort]->print(actionArray);
              myDebug[debugPort]->print(F("]"));
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
                    myDebug[debugPort]->println(F(".actionEnabled is Enabled"));
                  }
                  
                }else{
                  action[actionArray].actionEnabled = FALSE;
                  
                  if(setDebug & actionDebug)
                  {
                    myDebug[debugPort]->println(F(".actionEnabled is Disabled"));
                  }
                }
                
              if(setDebug & actionDebug)
                {
                  myDebug[debugPort]->print(F("action["));
                  myDebug[debugPort]->print(actionArray);
                  myDebug[debugPort]->print(F("].actionEnabled = "));
                  myDebug[debugPort]->println(action[actionArray].actionEnabled);
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
                    myDebug[debugPort]->print(F(".tooCold is set to "));
                    myDebug[debugPort]->println(actionEnableTemp);
                  }
                  
                }else if( actionSection == 3){
                  action[actionArray].tooHot = actionEnableTemp;
                  
                  if(setDebug & actionDebug)
                  {
                    myDebug[debugPort]->print(F(".tooHot is set to "));
                    myDebug[debugPort]->println(actionEnableTemp);
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
                myDebug[debugPort]->print(F("Case 3: result = "));
                myDebug[debugPort]->println(result);
              }
              actionDelayVal = ((uint32_t) atoi(result));
              
              if(setDebug & actionDebug)
              {
                myDebug[debugPort]->print(F("actionDelayVal = "));
                myDebug[debugPort]->println(actionDelayVal);
              }
              
              actionDelayVal *= 1000;
              
              if(setDebug & actionDebug)
              {
                myDebug[debugPort]->print(F("actionDelayVal * 1000 = "));
                myDebug[debugPort]->println(actionDelayVal);
                myDebug[debugPort]->print(F("action["));
                myDebug[debugPort]->print(actionArray);
                myDebug[debugPort]->print(F("]."));
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
                  myDebug[debugPort]->print(F("tcDelay = "));
                  myDebug[debugPort]->println((actionDelayVal / 1000));
                }
                
              }else if (actionSection == 3){
                action[actionArray].thDelay = actionDelayVal;
                if(actionDelayVal > 0)
                {
                  action[actionArray].thMillis = millis();
                }
                
                if(setDebug & actionDebug)
                {
                  myDebug[debugPort]->print(F("thDelay = "));
                  myDebug[debugPort]->println(actionDelayVal / 1000);
                }
              }
            }
            break;
          }
          
          case 5:
          {
            if(setDebug & actionDebug)
            {
              myDebug[debugPort]->print(F("Case 5 addrResult = "));
              myDebug[debugPort]->println(result);
            }
            addrResult = strtok( result, addrDelim );
            while(addrResult != NULL)
            {
              addrVal[addrResultCnt] = (uint8_t) strtol(addrResult, NULL, 16);
              
              if(setDebug & actionDebug)
              {
                myDebug[debugPort]->print(F(" "));
                myDebug[debugPort]->print(addrVal[addrResultCnt], HEX);
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
              myDebug[debugPort]->println();
              myDebug[debugPort]->print(F("chipAddrCnt =  "));
              myDebug[debugPort]->println(chipAddrCnt, HEX);
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
                myDebug[debugPort]->print(F("Case 4 LCD = "));
                myDebug[debugPort]->println(result);
              }
              action[actionArray].lcdAddr = atoi(result);
              if(setDebug & actionDebug)
              {
                myDebug[debugPort]->print(F("action["));
                myDebug[debugPort]->print(actionArray);
                myDebug[debugPort]->print(F("].lcdAddr = "));
                myDebug[debugPort]->println(action[actionArray].lcdAddr);
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
          lcd1w[x].Addr[cAcnt] = 0x00;
          glcd1w[x].Addr[cAcnt] = 0x00;
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
      
      chipCnt = 0x0;
      numberOfGLCDs = 0x0;
      numberOf1wLCDs = 0x0;

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
        myDebug[debugPort]->println(F("masterPidStop Enter"));
      }
      
      for(x=0;x<maxPIDs;x++)
      {
        ePID[x].pidEnabled = FALSE;
        
        if(setDebug & pidDebug)
        {
          myDebug[debugPort]->print(F("ePID["));
          myDebug[debugPort]->print(x);
          myDebug[debugPort]->println(F("].pidEnabled set to FALSE"));
        }
        
        ePID[x].myPID->SetMode(MANUAL);
        
        if(setDebug & pidDebug)
        {
          myDebug[debugPort]->print(F("ePID["));
          myDebug[debugPort]->print(x);
          myDebug[debugPort]->println(F("].myPID->SetMode() set to MANUAL"));
        }
        
        if(&ePID[x].switchPtr->chipAddr != NULL)
        {
          actionSwitchSet((uint8_t *) &ePID[x].switchPtr->chipAddr, ds2406PIOAoff);
          if(setDebug & pidDebug)
          {
              myDebug[debugPort]->print(F("ePID["));
              myDebug[debugPort]->print(x);
              myDebug[debugPort]->println(F("].switchPtr->chipAddr set to OFF"));
          }
        }
      }
      
      if(setDebug & pidDebug)
      {
        myDebug[debugPort]->println(F("masterPidStop Exit"));
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
        myDebug[debugPort]->println(F("updatePidArray Enter"));
        myDebug[debugPort]->println(PacketBuffer);
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
        myDebug[debugPort]->print(F("pidArray = "));
        myDebug[debugPort]->println(pidArray);
        myDebug[debugPort]->print(F("pidEnabledPtr = "));
        myDebug[debugPort]->println(pidEnabledPtr);
        myDebug[debugPort]->print(F("pidTempAddrPtr = "));
        myDebug[debugPort]->println(pidTempAddrPtr);
        myDebug[debugPort]->print(F("pidSetPointPtr = "));
        myDebug[debugPort]->println(pidSetPointPtr);
        myDebug[debugPort]->print(F("pidSwitchAddrPtr = "));
        myDebug[debugPort]->println(pidSwitchAddrPtr);
        myDebug[debugPort]->print(F("pidKpPtr = "));
        myDebug[debugPort]->println(pidKpPtr);
        myDebug[debugPort]->print(F("pidKiPtr = "));
        myDebug[debugPort]->println(pidKiPtr);
        myDebug[debugPort]->print(F("pidKdPtr = "));
        myDebug[debugPort]->println(pidKdPtr);
        myDebug[debugPort]->print(F("pidDirectionPtr = "));
        myDebug[debugPort]->println(pidDirectionPtr);
        myDebug[debugPort]->print(F("pidWindowSizePtr = "));
        myDebug[debugPort]->println(pidWindowSizePtr);
      }
  
      if(setDebug & pidDebug)
      {
        myDebug[debugPort]->print(F("pidEnabled = "));
        myDebug[debugPort]->println(ePID[pidArray].pidEnabled);
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
        myDebug[debugPort]->print(F("chipAddrCnt = "));
        myDebug[debugPort]->println(chipAddrCnt);
      }
      
      if(chipAddrCnt > chipCnt)
      {
        ePID[pidArray].tempPtr = NULL;
      }else{
        ePID[pidArray].tempPtr = &chip[chipAddrCnt];
      }
      
      if(setDebug & pidDebug)
      {
        myDebug[debugPort]->print(F("tempPtr = "));
        myDebug[debugPort]->println((uint32_t) ePID[pidArray].tempPtr, HEX);
      }

      ePID[pidArray].pidSetPoint = strtod(pidSetPointPtr, NULL);
      
      if(setDebug & pidDebug)
      {
        myDebug[debugPort]->print(F("pidSetPoint = "));
        myDebug[debugPort]->println((double) ePID[pidArray].pidSetPoint);
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
        myDebug[debugPort]->print(F("chipAddrCnt = "));
        myDebug[debugPort]->println(chipAddrCnt);
      }
      
      if(chipAddrCnt > chipCnt)
      {
        ePID[pidArray].switchPtr = NULL;
      }else{
        ePID[pidArray].switchPtr = &chip[chipAddrCnt];
      }
      
      if(setDebug & pidDebug)
      {
        myDebug[debugPort]->print(F("switchPtr = "));
        myDebug[debugPort]->println((uint32_t) ePID[pidArray].switchPtr, HEX);
      }

      ePID[pidArray].pidKp = strtod(pidKpPtr, NULL);
      
      if(setDebug & pidDebug)
      {
        myDebug[debugPort]->print(F("pidKp = "));
        myDebug[debugPort]->println((double) ePID[pidArray].pidKp);
      }

      ePID[pidArray].pidKi = strtod(pidKiPtr, NULL);
      
      if(setDebug & pidDebug)
      {
        myDebug[debugPort]->print(F("pidKi = "));
        myDebug[debugPort]->println((double) ePID[pidArray].pidKi);
      }

      ePID[pidArray].pidKd = strtod(pidKdPtr, NULL);
      
      if(setDebug & pidDebug)
      {
        myDebug[debugPort]->print(F("pidKd = "));
        myDebug[debugPort]->println((double) ePID[pidArray].pidKd);
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
        myDebug[debugPort]->print(F("pidDirection = "));
        myDebug[debugPort]->println(ePID[pidArray].pidDirection);
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
        myDebug[debugPort]->print(F("pidEnabled = "));
        myDebug[debugPort]->println(ePID[pidArray].pidEnabled);
      }
      
      if(setDebug & pidDebug)
      {
        myDebug[debugPort]->print(F("pidWindowSize = "));
        myDebug[debugPort]->println((double) ePID[pidArray].pidWindowSize);
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
    
    case useDebug: // "P" Set Debug Value
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
        myDebug[debugPort]->println(F("updateChipName Enter"));
        myDebug[debugPort]->println(PacketBuffer);
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
        myDebug[debugPort]->print(F("chip["));
        myDebug[debugPort]->print(chipAddrCnt);
        myDebug[debugPort]->print(F("].chipName = "));
        myDebug[debugPort]->println(chip[chipAddrCnt].chipName);
        for(cnCnt = 0; cnCnt < chipNameSize; cnCnt++)
        {
          myDebug[debugPort]->print(F("0x"));
          if(chip[chipAddrCnt].chipName[cnCnt] < 0x0f)
          {
            myDebug[debugPort]->print(F("0"));
          }
          myDebug[debugPort]->print(chip[chipAddrCnt].chipName[cnCnt], HEX);
          if(cnCnt < chipNameSize - 1)
          {
            myDebug[debugPort]->print(F(", "));
          }
        }
        myDebug[debugPort]->println();
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
      I2CEEPROM_readAnything(I2CEEPROMnumGLCDsAddr, numberOfGLCDs, I2C0x50);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d", numberOfGLCDs);
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
        if(setDebug & glcdDebug)
        {
          myDebug[debugPort]->print(F("glcd1w["));
          myDebug[debugPort]->print(x);
          myDebug[debugPort]->print(F("].Name = "));
          myDebug[debugPort]->println(glcd1w[x].Name);
        }

        showChipAddress((uint8_t *) &glcd1w[x].Addr);

        if( (glcd1w[x].Name[0] == ' ') || (glcd1w[x].Name[0] == 0x00) )
        {
          rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, ";%s", "__UNASSIGNED___");
        }else{
          rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, ";%s", glcd1w[x].Name);
        }

        rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, ";%02X",glcd1w[x].Flags);
        
        for( uint8_t ActionCnt = 0; ActionCnt < maxGLCDactions; ActionCnt++)
        {
          if(glcd1w[x].Action[ActionCnt] == NULL)
          {
            rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, ";%s","NULL");
          }else{
            rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, ";%08X",glcd1w[x].Action[ActionCnt]);
          }
        }

        for( uint8_t ChipCnt = 0; ChipCnt < maxGLCDchips; ChipCnt++)
        {
          if(glcd1w[x].Chip[ChipCnt] == NULL)
          {
            rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, ";%s","NULL");
          }else{
            rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, ";%08X",glcd1w[x].Chip[ChipCnt]);
          }
        }
      }

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
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d,%d,%d %08X", x, glcd1w[x].Flags, glcdPosition, glcd1w[x].Action[glcdPosition]);
      }else{
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "INVALID");
      }
      sendUDPpacket();
      break;
    }

    case getStructAddr: // "Z"/Type/Number
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
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%08X", &action[x]);
          }
          break;
        }
        
        case 'C': //chips
        {
          if(x > maxChips)
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "INVALID - Chip Too Large");
          }else{
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%08X", &chip[x]);
          }
          break;
        }
        
        case 'G': //1-wire GLCDs
        {
          if(x > maxGLCDs)
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "INVALID - GLCD Too Large");
          }else{
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%08X", &glcd1w[x]);
          }
          break;
        }
        
        case 'L': //1-wire LCDs
        {
          if(x > maxLCDs)
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "INVALID - LCD Too Large");
          }else{
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%08X", &lcd1w[x]);
          }
          break;
        }
        
        case 'P': //PIDs
        {
          if(x > maxPIDs)
          {
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "INVALID - PID Too Large");
          }else{
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%08X", &ePID[x]);
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

    case updateGLCD1wName: // "a" ADDRESS NAME
    {
      uint8_t cnCnt;
      
      if(setDebug & glcdNameUpdate)
      {
        myDebug[debugPort]->println(F("updateglcd1wName Enter - PacketBuffer = "));
        myDebug[debugPort]->print(PacketBuffer);
        myDebug[debugPort]->println();
      }
      
      result = strtok( PacketBuffer, delim );
      char* glcd1wNameAddr      = strtok( NULL, delim );
      char* glcd1wNameStr       = strtok( NULL, delim ); 
     
      if(setDebug & glcdNameUpdate)
      {
        myDebug[debugPort]->print(F("glcd1wNameAddr = "));
        myDebug[debugPort]->print(glcd1wNameAddr);
        myDebug[debugPort]->println();
        myDebug[debugPort]->print(F("glcd1wNameStr = "));
        myDebug[debugPort]->print(glcd1wNameStr);
        myDebug[debugPort]->println();
      }
      
      asciiArrayToHexArray(glcd1wNameAddr, addrDelim, addrVal);
      uint8_t glcd1wAddrCnt = matchAddress(addrVal, (uint8_t*) glcd1w, sizeof(glcd1wStruct), maxGLCDs);
//      uint8_t glcd1wAddrCnt = matchglcd1wAddress(addrVal);
      
      if(setDebug & glcdNameUpdate)
      {
        myDebug[debugPort]->print(F("glcd1wAddrCnt = "));
        myDebug[debugPort]->print(glcd1wAddrCnt);
        myDebug[debugPort]->println();
      }
      
      if((glcd1wAddrCnt >= 0) && (glcd1wAddrCnt < maxGLCDs))
      {
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
  
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "glcd[%d] Name Updated to %s", glcd1wAddrCnt, glcd1w[glcd1wAddrCnt].Name);
      }else{
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "No Match");
      }
      
      if(setDebug & glcdNameUpdate)
      {
        myDebug[debugPort]->print(ReplyBuffer);
        myDebug[debugPort]->println();
      }
      
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
/*
  case setDebugPort: // "c" Set Debug Port to IDE Serial Monitor or FTDI Serial2
    {
      x = atoi((char *) &PacketBuffer[1]);
      if(x == 2)
      {
        Serial.end();
        Serial2.begin(baudRate);
        Stream &myDebug = Serial2;
        myDebug[debugPort]->println("myDebug set to Serial2");
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "myDebug set to Serial2");
      }else if(x == 1){
        Serial2.end();
        Serial.begin(baudRate);
        Stream &myDebug = Serial;
        myDebug[debugPort]->println("myDebug set to Serial");
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "myDebug set to Serial");
      }else{
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "No Change");
      }
      sendUDPpacket();
      break;
    }
*/

  case setDebugPort: // "c" Set Debug Port to IDE Serial Monitor or FTDI Serial2
    {
      x = atoi((char *) &PacketBuffer[1]);
      debugPort = x;
      
      switch(x)
      {
        case 0:
        {
          myDebug[debugPort]->println(F("myDebug set to Serial"));
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "myDebug set to Serial");
          break;
        }
        
        case 1:
        {
          myDebug[debugPort]->println(F("myDebug set to Serial2"));
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "myDebug set to Serial2");
          break;
        }
        
        default:
        {
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "No Change");
          break;
        }
      }
      sendUDPpacket();
      break;
    }


/*
  case setDebugPort: // "c" Set Debug Port to IDE Serial Monitor or FTDI Serial2
    {
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "Not Implemented");
      sendUDPpacket();
      break;
    }
*/

    case getMaxLCDs: // "d"
    {
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d", maxLCDs);
      sendUDPpacket();
      break;
    }
    
    case getLCDcnt: // "e"
    {
      I2CEEPROM_readAnything(I2CEEPROMnum1wLCDsAddr, numberOf1wLCDs, I2C0x50);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d", numberOf1wLCDs);
      sendUDPpacket();
      break;
    }
    
    case getLCDstatus: // "f"
    {
      x = atoi((char *) &PacketBuffer[1]);
     if(x >= maxLCDs)
      {
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "ERROR");
      }else{
        showChipAddress((uint8_t *) &lcd1w[x].Addr);

        rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, ";%s", "L");

        if( (lcd1w[x].Name[0] == ' ') || (lcd1w[x].Name[0] == 0x00) )
        {
          rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, ";%s", "__UNASSIGNED___ ");
        }else{
          rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, ";%s", lcd1w[x].Name);
        }

        rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, ";%02X",lcd1w[x].Flags);
        
        if(lcd1w[x].Action == NULL)
        {
          rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, ";%s", "NULL");
        }else{
          rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, ";%08X",lcd1w[x].Action);
        }
        for(uint8_t rows = 0; rows < lcdRows; rows++)
        { 
          if(lcd1w[x].Chip[rows] == NULL)
          {
            rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, ";%s", "NULL");
          }else{
            rBuffCnt += sprintf(ReplyBuffer + rBuffCnt, ";%08X",lcd1w[x].Chip[rows]);
          }
        }
      }

      if(setDebug & debugLCD)
      {
        myDebug[debugPort]->print(F("lcd1w["));
        myDebug[debugPort]->print(x);
        myDebug[debugPort]->print(F("].Name = "));
        myDebug[debugPort]->println(ReplyBuffer);
      }
      sendUDPpacket();
      break;
    }

    case setLCD: // "g" 1wLCD FLAGS ACTION CHIP
    {
      if(PacketBuffer[1] == ' ')
      {
        result = strtok( PacketBuffer, delim );
        char* dev       = strtok( NULL, delim );
        char* flags     = strtok( NULL, delim );
        char* actionNum = strtok( NULL, delim );
        char* chipNum   = strtok( NULL, delim );
      
        x = atoi(dev);
        lcd1w[x].Flags = atoi(flags);
        uint32_t Action = strtoul(actionNum, NULL, 16);
        uint32_t Chip   = strtoul(chipNum, NULL, 16);
        if(Action != NULL)
        {
          lcd1w[x].Action = (chipActionStruct *) Action;
        }else{
          lcd1w[x].Action = (chipActionStruct *) NULL;
        }
        for(uint8_t rows = 0; rows < lcdRows; rows ++)
        {
          if(Chip != NULL)
          {
            lcd1w[x].Chip[rows] = (chipStruct *) Chip;
          }else{
            lcd1w[x].Chip[rows] = (chipStruct *) NULL;
          }
        }
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X", 
                            x, lcd1w[x].Action, lcd1w[x].Chip[0], lcd1w[x].Chip[1], lcd1w[x].Chip[2], lcd1w[x].Chip[3]);
      }else{
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "INVALID");
      }
      sendUDPpacket();
      break;
    }

    case updateLCD1wName: // "h" ADDRESS NAME
    {
      uint8_t cnCnt;
      
      for(uint16_t x = 0; x < sizeof(PacketBuffer); x++) // strip out CR/LF
        if((PacketBuffer[x] == 0x0D) || (PacketBuffer[x] == 0x0A))
          PacketBuffer[x] = 0x00;
          
      if(setDebug & lcdNameUpdate)
      {
        myDebug[debugPort]->print(F("updatelcd1wName Enter - PacketBuffer = "));
        myDebug[debugPort]->println(PacketBuffer);
      }
      
      result = strtok( PacketBuffer, delim );
      char* lcd1wNameAddr      = strtok( NULL, delim );
      char* lcd1wNameStr       = strtok( NULL, delim ); 
     
      if(setDebug & lcdNameUpdate)
      {
        myDebug[debugPort]->print(F("lcd1wNameAddr = "));
        myDebug[debugPort]->print(lcd1wNameAddr);
        myDebug[debugPort]->println();
        myDebug[debugPort]->print(F("lcd1wNameStr = "));
        myDebug[debugPort]->print(lcd1wNameStr);
        myDebug[debugPort]->println();
      }
      
      asciiArrayToHexArray(lcd1wNameAddr, addrDelim, addrVal);
      
      if(setDebug & lcdNameUpdate)
      {
        myDebug[debugPort]->print(F("addrVal = "));      
        for(uint8_t x = 0; x < chipAddrSize; x++)
        {
          myDebug[debugPort]->print(F("0x"));
          if(addrVal[x] < 0x0F)
            myDebug[debugPort]->print(F("0"));
          myDebug[debugPort]->print(addrVal[x], HEX);
          if(x < (chipAddrSize - 1))
            myDebug[debugPort]->print(F(","));
        }
        myDebug[debugPort]->println(); 
        myDebug[debugPort]->println(F("Entering matchlcd1wAddress"));
      }
/*
      uint8_t lcd1wAddrCnt = matchlcd1wAddress(addrVal);
      myDebug[debugPort]->print(F("lcd1wAddrCnt = "));
      myDebug[debugPort]->println(lcd1wAddrCnt);
*/
      uint8_t lcd1wAddrCnt = matchAddress(addrVal, (uint8_t*) lcd1w, sizeof(t3LCDStruct), maxLCDs);
      myDebug[debugPort]->print(F("lcd1wAddrCnt = "));
      myDebug[debugPort]->println(lcd1wAddrCnt);

      
      if(setDebug & lcdNameUpdate)
      {
        myDebug[debugPort]->println(F("Exiting matchlcd1wAddress"));
        myDebug[debugPort]->print(F("lcd1wAddrCnt = "));
        myDebug[debugPort]->println(lcd1wAddrCnt);
      }
      
      if((lcd1wAddrCnt >= 0) && (lcd1wAddrCnt < maxLCDs) )
      {
        for(cnCnt = 0; cnCnt < chipNameSize; cnCnt++)  // clear the name array
        {
          lcd1w[lcd1wAddrCnt].Name[cnCnt] = 0x00;
        }
        
        strcpy(lcd1w[lcd1wAddrCnt].Name, lcd1wNameStr); //copy the name array
  
        for(cnCnt = 0; cnCnt < chipNameSize; cnCnt++)
        {
          if(lcd1w[lcd1wAddrCnt].Name[cnCnt] < 0x30 || lcd1w[lcd1wAddrCnt].Name[cnCnt] > 0x7A)
          {
            lcd1w[lcd1wAddrCnt].Name[cnCnt] = 0x00; //remove non-ascii characters
          }
        }
  
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "lcd1w[%d] Name Updated to %s",lcd1wAddrCnt, lcd1w[lcd1wAddrCnt].Name);
      }else{
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", "No Match");
      }
      
      if(setDebug & lcdNameUpdate)
      {
        myDebug[debugPort]->print(ReplyBuffer);
        myDebug[debugPort]->println();
      }
      
      sendUDPpacket();
      if(setDebug & lcdNameUpdate)
      {
        myDebug[debugPort]->println(F("updatelcd1wName Exit"));
      }
      break;
    }
    

  case resetLCD: // "i"
    {
      x = atoi((char *) &PacketBuffer[1]);
      resetLCDdisplay(x);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s%d Reset","LCD", x);
      sendUDPpacket();
      break;
    }

  case getDebug: // "p"
    {
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "0x%08X", setDebug);
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

  case setTempType: // "t" Set Temp display to F or C
    {
      x = atoi((char *) &PacketBuffer[1]);
      if(x == 0)
      {
        showCelsius = TRUE;
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","Setting Celsius");
      }else if(x == 1){
        showCelsius = FALSE;
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","Setting Fahrenheit");
      }else{
        rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s","Invalid Value");
      }
      sendUDPpacket();
      break;
    }

    case displayMessage: // "w"
    {
      /*********************
      Display LCD String format is:
      
      lcdTypeStr lcdDisplayStr lcdLineCtr clrLcdStr lcdMessage
      
      where:
      lcdTypeStr is the type of LCD display, either 1-wire (1) or I2C (2)
      lcdDisplayStr is the address of the display, currently 0 - 7 for I2C or 0 - 12 for 1-wire
      lcdLineCtr is the line on which to put the message 0 - 3
      clrLcdStr clears the entire display
      lcdMessage is the message string, currently 20 characters, more than 20 results in an error
      
      underscores are converted to spaces, line and carriage returns are zeroed out. 
      
      *********************/
      result = strtok( PacketBuffer, delim );

      char* lcdTypeStr     = strtok( NULL, delim );
      char* lcdDisplayStr  = strtok( NULL, delim );
      char* lcdLineCtr     = strtok( NULL, delim );
      char* clrLcdStr      = strtok( NULL, delim );
      char* lcdMessage     = strtok( NULL, delim );
      
      uint8_t lcdType    = atoi(lcdTypeStr);
      uint8_t lcdDisplay = atoi(lcdDisplayStr);
      uint8_t lcdLine    = atoi(lcdLineCtr);
      uint8_t clrLCD     = atoi(clrLcdStr);
      
      uint8_t msgLength = strlen(lcdMessage);
      
      if(setDebug & lcdDebug)
      {
        myDebug[debugPort]->print(F("lcdMessage is "));
        myDebug[debugPort]->println(lcdMessage);
        for( int t = 0; t < msgLength; t++)
        {
          myDebug[debugPort]->print(F("0x"));
          if(lcdMessage[t] < 0x10)
          {
            myDebug[debugPort]->print(F("0"));
          }
          myDebug[debugPort]->print(lcdMessage[t], HEX);
          if(t < (msgLength - 1))
          {
            myDebug[debugPort]->print(F(", "));
          }
        }
        myDebug[debugPort]->println();
      }
      
      if( msgLength > 21 )
      {
        if(setDebug & lcdDebug)
        {
          myDebug[debugPort]->print(F("lcdMessage is "));
          myDebug[debugPort]->print(msgLength);
          myDebug[debugPort]->println(F(" too long, truncating"));
        }
        lcdMessage[20] = 0x0;
      }
      
      if(clrLCD == 1)
      {
        switch(lcdType)
        {
          case 1: //1-wire
          {
            if(lcd1w[lcdDisplay].Addr[0] == dsLCD)
            {
              sendLCDcommand(lcdDisplay, clr1wLCD, 0, 0, nullStr, 1);
              rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s %d %s", "OK - 1-wire display", lcdDisplay, "cleared ");
            }else{
              rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d %s", lcdDisplay, "Invalid 1-wire address");
            }
            break;
          }
          
          case 2: // I2C
          {
            lcd[lcdDisplay]->clear();
            lcd[lcdDisplay]->home();
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s %d %s", "OK - I2C display", lcdDisplay, "cleared ");
            break;
          }
        }
      }

      for(int s = 0; s < lcdChars; s++)
      {
        switch(lcdMessage[s])
        {
          case '_': //convert "_" to spaces
          {
            lcdMessage[s] = ' ';
            break;
          }
          
          case 0x0A: //convert 0xOA or 0X0D to zero string terminator
          case 0x0D:
          {
            lcdMessage[s] = 0x00;            
            break;
          }
        }
      }

      switch(lcdType)
      {
        case 1: // 1-wire
        {
          if(lcd1w[lcdDisplay].Addr[0] == dsLCD)
          {
            sendLCDcommand(lcdDisplay, prt1wLCD, lcdLine, 0, lcdMessage, 1);
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s %d %s %s", "OK - 1-wire display", lcdDisplay, lcdMessage, "Message Sent");
          }else{
            rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d %s", lcdDisplay, "Invalid 1-wire address");
          }
          break;
        }
        
        case 2: // I2C
        {
          lcd[lcdDisplay]->setCursor(0, lcdLine);
          lcd[lcdDisplay]->print(lcdMessage);
          rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s %d %s %s", "OK - I2C display", lcdDisplay, lcdMessage, "Message Sent");
          break;
        }
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
        myDebug[debugPort]->println(F("MasterStop() completed"));
        delay(1000);
      }
      
      setLED(LED1, ledON);
//      digitalWrite(LED1, LOW); // indicate that I2CEEPROM is being accessed
      EEPROMclear();

      if(setDebug & resetDebug)
      {
        myDebug[debugPort]->println(F("EEPROMclear() completed"));
      }
      I2CEEPROM_readAnything(I2CEEPROMidAddr, i2cEeResult, I2C0x50);
  
      if(setDebug & resetDebug)
      { 
        myDebug[debugPort]->print(F("i2cEeResult = 0x"));
        myDebug[debugPort]->println(i2cEeResult, HEX);
      }
  
      if(i2cEeResult != 0x55)
      {
        if(setDebug & resetDebug)
        { 
          myDebug[debugPort]->println(F("No EEPROM Data"));
          delay(1000);
        }
      }
        
      for(x = 0; x < maxChips; x++)
      {
        memcpy(&chip[x], &chipClear, sizeof(chipStruct));
      }
      
      if(setDebug & resetDebug)
      { 
        myDebug[debugPort]->println(F("chip structures cleared"));
        delay(1000);
      }
  
      for(x = 0; x < maxActions; x++)
      {
        memcpy(&action[x], &actionClear, sizeof(chipActionStruct));
      }
      
      if(setDebug & resetDebug)
      { 
        myDebug[debugPort]->println(F("action structures cleared"));
        delay(1000);
      }
  
      for(x = 0; x < maxPIDs; x++)
      {
        memcpy(&ePID[x], &pidClear, sizeof(chipPIDStruct));
      }
      
      if(setDebug & resetDebug)
      { 
        myDebug[debugPort]->println(F("pid structures cleared"));
        delay(1000);
      }
  
      setLED(LED1, ledOFF);
//      digitalWrite(LED1, HIGH); // indicate that I2CEEPROM is being accessed

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
    setLED(LED3, ledOFF);
//    digitalWrite(LED3, HIGH); // stop updProcess sync
  KickDog(); // reset Hardware Watchdog
}

void sendUDPpacket(void)
{
  int v = 0, q = 0;
  
  if(setDebug & udpDebug)
  {
    myDebug[debugPort]->print(F("rBuffCnt = "));
    myDebug[debugPort]->println(rBuffCnt);
    myDebug[debugPort]->println(F("ReplyBuffer:"));
    for(q = 0; q < rBuffCnt; q++)
    {
      if(ReplyBuffer[q] != 0x00)
      {
        myDebug[debugPort]->write(ReplyBuffer[q]);
        if(ReplyBuffer[q] == ';')
        {
          myDebug[debugPort]->println();
        }
      }
    }
    myDebug[debugPort]->println();
    if(setDebug & udpHexBuff)
    {
      for(q = 0; q < rBuffCnt; q++)
      {
        myDebug[debugPort]->print(F("0x"));
        if(ReplyBuffer[q] < 0x10)
        {
          myDebug[debugPort]->print(F("0"));
        }
        myDebug[debugPort]->print(ReplyBuffer[q], HEX);
        if(v <= 14)
        {
          myDebug[debugPort]->print(F(" "));
          v++;
        }else{
          myDebug[debugPort]->println();
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


