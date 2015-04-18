/********************

TeensyNet Action functions

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
        myDebug[debugPort]->print(F("tempStrCnt for "));
        myDebug[debugPort]->print(action[x].tempPtr->chipName);
        myDebug[debugPort]->print(F(" is "));
        myDebug[debugPort]->println(tempStrCnt);
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
        myDebug[debugPort]->print(F("tempStrCnt for action["));
        myDebug[debugPort]->print(x);
        myDebug[debugPort]->print(F("]tempPtr.->chipStatus"));
        myDebug[debugPort]->print(F(" is "));
        myDebug[debugPort]->print(tempStrCnt);
        myDebug[debugPort]->print(F(" and the chipStatus is "));
        myDebug[debugPort]->println((int16_t) action[x].tempPtr->chipStatus);
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
          myDebug[debugPort]->print(F("tempStrCnt for "));
          myDebug[debugPort]->print(action[x].tcPtr->chipName);
          myDebug[debugPort]->print(F(" is "));
          myDebug[debugPort]->println(tempStrCnt);
        }else{
          myDebug[debugPort]->print(F("action["));
          myDebug[debugPort]->print(x);
          myDebug[debugPort]->println(F("].tcPtr->chipName is not used"));
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
          myDebug[debugPort]->print(F("tempStrCnt for "));
          myDebug[debugPort]->print(action[x].thPtr->chipName);
          myDebug[debugPort]->print(F(" is "));
          myDebug[debugPort]->println(tempStrCnt);
        }else{
          myDebug[debugPort]->print(F("action["));
          myDebug[debugPort]->print(x);
          myDebug[debugPort]->println(F("].thPtr->chipName is not used"));
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
        myDebug[debugPort]->print(F("lcdupdate took "));
        myDebug[debugPort]->print(lcdUpdateStop - lcdUpdateStart);
        myDebug[debugPort]->println(F(" milliseconds"));
      }

    }
  }
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

void actionSwitchSet(uint8_t* array, uint8_t setChipState)
{
   uint8_t chipAddrCnt;

  chipAddrCnt = matchChipAddress(array);
  
  if(chipAddrCnt != 0xFF)
  {
    setSwitch(chipAddrCnt, setChipState);
  }
}



