/********************

TeensyNet 1-wire 4x20 HD47780-compatible LCD functions

Version 0.0.50
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

/**** Begin TeensyNetLCD functions ****/
void sendLCDcommand(uint8_t device, uint8_t t3LCDcmd, uint8_t row,
                    uint8_t col, char *lcdStr,  uint8_t t3LCDrdy
                   )
{
  uint8_t present = 0;

  t3LCDunion.t3LCDbuf.t3LCDcmd = t3LCDcmd;
  t3LCDunion.t3LCDbuf.row = row;
  t3LCDunion.t3LCDbuf.col = col;
  sprintf(t3LCDunion.t3LCDbuf.lcdStr, "%s", lcdStr);
  t3LCDunion.t3LCDbuf.t3LCDrdy = 1;
  
  
  if(setDebug & lcd1wLED)
    setLED(LED5, ledON);
//    digitalWrite(LED5, LOW);   // turn them LED5 on
  ds.reset();
  ds.select(lcd1w[device].Addr);
  ds.write(0x0F);                     // Write to LCD
  ds.write_bytes(t3LCDunion.lcdArray, sizeof(t3LCDunion));
 // delay(100);
  if(setDebug & lcd1wLED)
    setLED(LED5, ledOFF);
//    digitalWrite(LED5, HIGH);   // turn them LED5 off
  
  if(setDebug & lcdSerialDebug)
  {
    myDebug[debugPort]->print(F("t3LCDunion.t3LCDbuf.t3LCDcmd = "));
    myDebug[debugPort]->println(t3LCDunion.t3LCDbuf.t3LCDcmd);
    myDebug[debugPort]->print(F("t3LCDunion.t3LCDbuf.row = "));
    myDebug[debugPort]->println(t3LCDunion.t3LCDbuf.row);
    myDebug[debugPort]->print(F("t3LCDunion.t3LCDbuf.col = "));
    myDebug[debugPort]->println(t3LCDunion.t3LCDbuf.col);
    myDebug[debugPort]->print(F("t3LCDunion.t3LCDbuf.lcdStr = "));
    myDebug[debugPort]->println(t3LCDunion.t3LCDbuf.lcdStr);
    myDebug[debugPort]->print(F("t3LCDunion.t3LCDbuf.t3LCDrdy = "));
    myDebug[debugPort]->println(t3LCDunion.t3LCDbuf.t3LCDrdy);
  }
  delay(40);
}

void updateLCD1W(int cnt, uint8_t x)
{
  if(setDebug & lcdSerialDebug)
  {
    myDebug[debugPort]->print(F("Starting updateLCD1W - cnt = "));
    myDebug[debugPort]->print(cnt);
    myDebug[debugPort]->print(F(", x = "));
    myDebug[debugPort]->println(x);
  }
  for(uint8_t y = 0; y < chipAddrSize; y++)
  {
    lcd1w[cnt].Addr[y] = chip[x].chipAddr[y]; // move the address to the GLCD 1-wire array
    chip[x].chipAddr[y] = 0; // reset the array
  }

  char str1[lcdCols+1];

// initialize the lcd structure and device

  sendLCDcommand(cnt, clr1wLCD, 0, 0, nullStr, 1);
  if(setDebug & lcdSerialDebug)
  {
    myDebug[debugPort]->print(F("Sent to device "));
    myDebug[debugPort]->print(x);
    myDebug[debugPort]->print(F(", Display "));
    myDebug[debugPort]->print(cnt);
    myDebug[debugPort]->println(F(" - clrLCD"));
  }
  
  sprintf(str1, "Display %d", cnt);
  lcdCenterStr(str1, lcdCols);
  sendLCDcommand(cnt, prt1wLCD, 0, 0, lcdStr, 1);
  if(setDebug & lcdSerialDebug)
  {
    myDebug[debugPort]->print(F("Sent to device "));
    myDebug[debugPort]->print(x);
    myDebug[debugPort]->print(F(", Display "));
    myDebug[debugPort]->print(cnt);
    myDebug[debugPort]->print(F(", row 0 - "));
    myDebug[debugPort]->println(str1);
  }

  sprintf(str1, "0x%02X,0x%02X,0x%02X,0x%02X", 
          lcd1w[cnt].Addr[0],lcd1w[cnt].Addr[1],lcd1w[cnt].Addr[2],lcd1w[cnt].Addr[3]);
  lcdCenterStr(str1, lcdCols);
  sendLCDcommand(cnt, prt1wLCD, 1, 0, lcdStr, 1);
  if(setDebug & lcdSerialDebug)
  {
    myDebug[debugPort]->print(F("Sent to device "));
    myDebug[debugPort]->print(x);
    myDebug[debugPort]->print(F(", Display "));
    myDebug[debugPort]->print(cnt);
    myDebug[debugPort]->print(F(", row 2 - "));
    myDebug[debugPort]->println(str1);
  }

  sprintf(str1, "0x%02X,0x%02X,0x%02X,0x%02X",
          lcd1w[cnt].Addr[4],lcd1w[cnt].Addr[5],lcd1w[cnt].Addr[6],lcd1w[cnt].Addr[7]);
  lcdCenterStr(str1, lcdCols);
  sendLCDcommand(cnt, prt1wLCD, 2, 0, lcdStr, 1);
  if(setDebug & lcdSerialDebug)
  {
    myDebug[debugPort]->print(F("Sent to device "));
    myDebug[debugPort]->print(x);
    myDebug[debugPort]->print(F(", Display "));
    myDebug[debugPort]->print(cnt);
    myDebug[debugPort]->print(F(", row 2 - "));
    myDebug[debugPort]->println(str1);
  }

  if(setDebug & lcdSerialDebug)
  {
    myDebug[debugPort]->print(F("LCD "));
    myDebug[debugPort]->print(cnt);
    myDebug[debugPort]->print(F(" Address 0x"));
    myDebug[debugPort]->print((uint32_t) &(lcd1w[cnt].Addr), HEX);
    myDebug[debugPort]->print(F(" = {"));

    for( int i = 0; i < chipAddrSize; i++)
    {
      if(lcd1w[cnt].Addr[i]>=0 && lcd1w[cnt].Addr[i]<16)
      {
        myDebug[debugPort]->print(F("0x0"));
      }else{
        myDebug[debugPort]->print(F("0x"));
      }
      myDebug[debugPort]->print(lcd1w[cnt].Addr[i], HEX);
      if(i < 7){myDebug[debugPort]->print(F(","));}
    }
    myDebug[debugPort]->println(F("}"));
  }
}

void resetLCDdisplay(uint8_t q)
{
  sendLCDcommand(q, clr1wLCD, 0, 0, nullStr, 1);  
}

void lcd1wUpdate(void)
{
  uint32_t startTime, endTime;
  char str1[lcdCols+1];

  if(setDebug & debugLCD)
  {
    myDebug[debugPort]->print(F("Testing LCD["));
    myDebug[debugPort]->print(lcdCnt);
    myDebug[debugPort]->println(F("]"));
  }
    
  
  if(lcd1w[lcdCnt].Flags & lcdFActive)
  {
    if(setDebug & debugLCD)
    {
      myDebug[debugPort]->print(F("LCD["));
      myDebug[debugPort]->print(lcdCnt);
      myDebug[debugPort]->println(F("] Active!!"));
    }
    
    if(lcd1w[lcdCnt].Action != NULL)
    {
      startTime = micros();
      lcdCenterStr(lcd1w[lcdCnt].Name, lcdCols);
      sendLCDcommand(lcdCnt, prt1wLCD, 0, 0, lcdStr, 1);
      lcdRightStr(lcd1w[lcdCnt].Action->tempPtr->chipName, chipNameSize);
      sprintf(&lcdStr[lcd1wTempLoc], "%4d%c", (int16_t)lcd1w[lcdCnt].Action->tempPtr->chipStatus, degreeChar);
      sendLCDcommand(lcdCnt, prt1wLCD, 1, 0, lcdStr, 1);
      lcdRightStr(lcd1w[lcdCnt].Action->tcPtr->chipName, chipNameSize);
      lcd1wSwitchStatus((int16_t) lcd1w[lcdCnt].Action->tcPtr->chipStatus, lcdStr);
      sendLCDcommand(lcdCnt, prt1wLCD, 2, 0, lcdStr, 1);
      lcdRightStr(lcd1w[lcdCnt].Action->thPtr->chipName, chipNameSize);
      lcd1wSwitchStatus((int16_t) lcd1w[lcdCnt].Action->thPtr->chipStatus, lcdStr);
      sendLCDcommand(lcdCnt, prt1wLCD, 3, 0, lcdStr, 1);      
      endTime = micros();
      if(setDebug & lcdDebug)
      {
        myDebug[debugPort]->print(F("Elapsed Time = "));
        myDebug[debugPort]->print(endTime - startTime);
        myDebug[debugPort]->println(F(" microseconds"));
      }
    }else{
      for(uint8_t rows = 0; rows < lcdRows; rows++)
      {
        if(lcd1w[lcdCnt].Chip[rows] != NULL)
        {
        }
      }
    }
  }
  if(++lcdCnt == maxLCDs)
    lcdCnt = 0x00;
}

void lcd1wSwitchStatus(int16_t switchStatus, char *str)
{
  switch(switchStatus)
  {
    case 'N':
    {
      sprintf(&str[lcd1SwitchLoc], "%s", " ON");
      break;
    }

    case 'F':
    {
      sprintf(&str[lcd1SwitchLoc], "%s", "OFF");
      break;
    }

    case 'Z':
    default:
    {
      sprintf(&str[lcd1SwitchLoc], "%s", "-Z-");
      break;
    }
  }
}
