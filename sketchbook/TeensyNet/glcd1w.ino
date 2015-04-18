/********************

TeensyNet 1-wire 7" CTE GLCD functions

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

/**** Begin TeensyNetGLCD functions ****/
void sendGLCDcommand(uint8_t deviceType, uint8_t device, uint32_t  flags ,
                 uint8_t font, uint8_t bGR, uint8_t cR,
                 uint8_t dispP, uint8_t lineP, uint16_t chrP, 
                 char *dSTR, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t rad, uint8_t bRDY)
{

  uint8_t x, i;

  if(setDebug & glcdDebug)
  {
    setLED(LED5, ledON);
//    digitalWrite(LED5, LOW);
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
      myDebug[debugPort]->print(F("GLCD Addr: "));
      myDebug[debugPort]->print(x);
      myDebug[debugPort]->print(F(" = "));
      for( i = 0; i < 8; i++)
      {
        myDebug[debugPort]->print(F("0x"));
        if(glcd1w[device].Addr[i] < 16) myDebug[debugPort]->print(F("0"));
        myDebug[debugPort]->print(glcd1w[device].Addr[i], HEX);
        myDebug[debugPort]->print(F(" "));
      }
      myDebug[debugPort]->println();
    }else{
      delayMicroseconds(50);
    }
    if( (setDebug & glcdSerialDebug) || (setDebug & glcdSerialDebug) )
    {
      setLED(LED4, ledON);
//      digitalWrite(LED4, LOW);
    }
    ds.reset();
    ds.select(glcd1w[device].Addr);
    ds.write(0x0F);                     // Write to GLCD
    ds.write_bytes(glcdUNION.glcdARRAY, sizeof(glcdCMD));
    if( (setDebug & glcdSerialDebug) || (setDebug & glcdSerialDebug) )
    {
      setLED(LED4, ledOFF);
//      digitalWrite(LED4, HIGH);
    }
    if(setDebug & glcdSerialDebug)
    {
      myDebug[debugPort]->print(F("DATA = "));
      for( i = 0; i < sizeof(glcdCMD); i++)
      {
        myDebug[debugPort]->print(F("0x"));
        if(glcdUNION.glcdARRAY[i] < 16) myDebug[debugPort]->print(F("0"));
        myDebug[debugPort]->print(glcdUNION.glcdARRAY[i], HEX);
        myDebug[debugPort]->print(F(" "));
      }
      myDebug[debugPort]->println();
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
    setLED(LED5, ledOFF);
//    digitalWrite(LED5, HIGH);
  }
}

void clearGLCDcommandBuf(void)
{
  for(uint8_t x = 0; x< sizeof(glcdCMD); x++)
  {
    glcdUNION.glcdARRAY[x] = 0;
  }
}

void glcd1WUpdate(void)
{
uint8_t color;
char tempStr[7]    = "      ";
char actionStr[16] = "               ";
char switchStr[7] = "UNUSED";

  if(setDebug & glcd1WLED)
    setLED(LED1, ledON);
//    digitalWrite(LED1, LOW);
    
  switch(glcd1w[glcd1wCnt].Addr[0])
  {
    case 0x45:
    {
      if(setDebug & glcdDebug)
        myDebug[debugPort]->println(F("GLCD Found!!"));
        
      if(glcd1w[glcd1wCnt].Flags & glcdFActive)
      {
        if(setDebug & glcdDebug)
          myDebug[debugPort]->println(F("GLCD Active!!"));
          
        switch(glcd1w[glcd1wCnt].Flags & (glcdFAction | glcdFChip))
        {
          case glcdFAction:
          {
            switch(glcd1w[glcd1wCnt].Item)
            {
              case glcdActionSetPage:
              {
                if(setDebug & glcdSerialDebug)
                {
                  myDebug[debugPort]->print(F("Setting Page Write to "));
                  myDebug[debugPort]->println(glcd1w[glcd1wCnt].Page);
                }
                sendGLCDcommand(dsDevice, glcd1wCnt, setPageWrite, 0, 0, 0, glcd1w[glcd1wCnt].Page, 0, 0, nullStr, 0, 0, 0, 0, 0, 1); // Set the page to write into
                glcd1w[glcd1wCnt].Item += 1;
                break;
              }

              case glcdDrawRectangle:
              {
                if(setDebug & glcdSerialDebug)
                {
                  myDebug[debugPort]->print(F("Placing Filled Rectangle"));
                }
                if( (glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position] == NULL) | 
                    (glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->tempPtr == NULL)
                  )
                {
                  color = YELLOW;
                }else if(glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->tempPtr->chipStatus <= glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->tooCold){
                  color = BLUE;
                }else if(glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->tempPtr->chipStatus >= glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->tooHot){
                  color = RED;
                }else{
                  color = LIME;
                }

                sendGLCDcommand(dsDevice, glcd1wCnt, (setColorVGA | setDrawFRRect | setBackVGA), 0,
                             BLACK, color,
                             0, 0, 0, nullStr,
                             action2x2[glcd1w[glcd1wCnt].Position].rectX1, action2x2[glcd1w[glcd1wCnt].Position].rectY1,
                             action2x2[glcd1w[glcd1wCnt].Position].rectX2,  action2x2[glcd1w[glcd1wCnt].Position].rectY2, 0, 1);
                
                glcd1w[glcd1wCnt].Item += 1;
                break;
              }

              case glcdPrintTempName:
              {
                if(setDebug & glcdDebug)
                {
                  sprintf(actionStr, "LCD %d - Page %d", glcd1wCnt, glcd1w[glcd1wCnt].Page);
                }else{
                  if( (glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position] == NULL) | 
                      (glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->tempPtr == NULL)
                    )
                  {
                    lcdCenterStr((char *)"UNNAMED", chipNameSize);
//                    sprintf(actionStr, "%s", "    UNNAMED    ");
                    sprintf(actionStr, "%s", lcdStr);
                  }else{
                    lcdCenterStr(glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->tempPtr->chipName, chipNameSize);
//                    sprintf(actionStr, "%s", glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->tempPtr->chipName);
                    sprintf(actionStr, "%s", lcdStr);
                  }
                }
                sendGLCDcommand(dsDevice, glcd1wCnt, (setColorVGA | setPrintStrXY | setFont),  action2x2[glcd1w[glcd1wCnt].Position].tFont,
                             0, WHITE,
                             0, 0, 0, actionStr,
                             action2x2[glcd1w[glcd1wCnt].Position].tX1, action2x2[glcd1w[glcd1wCnt].Position].tY1, 0, 0, 0, 1);
                glcd1w[glcd1wCnt].Item += 1;
                break;
              }

              case glcdPrintTemp:
              {
                if( (glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position] == NULL) | 
                    (glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->tempPtr == NULL)
                  )
                {
                  sprintf(tempStr, "%s", "------");
                }else{
                  sprintf(tempStr, "%4d%c", glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->tempPtr->chipStatus, '.');
                }
                lcdCenterStr(tempStr, 6);
                sprintf(tempStr, "%s", lcdStr);
                sendGLCDcommand(dsDevice, glcd1wCnt, (setColorVGA | setPrintStrXY | setFont),  action2x2[glcd1w[glcd1wCnt].Position].vFont,
                             0, WHITE,
                             0, 0, 0, tempStr,
                             action2x2[glcd1w[glcd1wCnt].Position].vX1, action2x2[glcd1w[glcd1wCnt].Position].vY1, 0, 0, 0, 1);
                glcd1w[glcd1wCnt].Item += 1;
                break;
              }

              case glcdPrintS1Name:
              {
                if(setDebug & glcdDebug)
                {
                  sprintf(actionStr, "Page %d - Pos %d", glcd1w[glcd1wCnt].Page, glcd1w[glcd1wCnt].Position);
                }else{
                  if( (glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position] == NULL) | 
                      (glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->tcPtr == NULL)
                    )
                  {
                    lcdCenterStr((char *)"UNNAMED", chipNameSize);
//                    sprintf(actionStr, "%s", "    UNNAMED    ");
                    sprintf(actionStr, "%s", lcdStr);
                  }else{
                    lcdCenterStr(glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->tcPtr->chipName, chipNameSize);
//                    sprintf(actionStr, "%s", glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->tempPtr->chipName);
                    sprintf(actionStr, "%s", lcdStr);
                  }
                }
                sendGLCDcommand(dsDevice, glcd1wCnt, (setColorVGA | setPrintStrXY | setFont),  action2x2[glcd1w[glcd1wCnt].Position].s1Font,
                             0, WHITE,
                             0, 0, 0, actionStr,
                             action2x2[glcd1w[glcd1wCnt].Position].s1X1, action2x2[glcd1w[glcd1wCnt].Position].s1Y1, 0, 0, 0, 1);
                glcd1w[glcd1wCnt].Item += 1;
                break;
              }

              case glcdPrintS1Val:
              {
                if( (glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position] == NULL) | 
                    (glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->tcPtr == NULL)
                  )
                {
                  sprintf(switchStr, "%s", "UNUSED");
                }else{
                  if(glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->tcPtr->chipStatus == 'N')
                  {
                    lcdCenterStr((char *)"ON", sizeof(switchStr));
                  }else{
                   lcdCenterStr((char *)"OFF", sizeof(switchStr));
                  }
                  sprintf(switchStr, "%s", lcdStr);
                }

                sendGLCDcommand(dsDevice, glcd1wCnt, (setColorVGA | setPrintStrXY | setFont),  action2x2[glcd1w[glcd1wCnt].Position].s2Font,
                             0, WHITE,
                             0, 0, 0, switchStr,
                             action2x2[glcd1w[glcd1wCnt].Position].s2X1, action2x2[glcd1w[glcd1wCnt].Position].s2Y1, 0, 0, 0, 1);
                glcd1w[glcd1wCnt].Item += 1;
                break;
              }

              case glcdPrintS2Name:
              {
                if(setDebug & glcdDebug)
                {
                  sprintf(actionStr, "Page %d - Pos %d", glcd1w[glcd1wCnt].Page, glcd1w[glcd1wCnt].Position);
                }else{
                  if( (glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position] == NULL) | 
                      (glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->thPtr == NULL)
                    )
                  {
                    lcdCenterStr((char *)"UNNAMED", chipNameSize);
//                    sprintf(actionStr, "%s", "    UNNAMED    ");
                    sprintf(actionStr, "%s", lcdStr);
                  }else{
                    lcdCenterStr(glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->thPtr->chipName, chipNameSize);
//                    sprintf(actionStr, "%s", glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->tempPtr->chipName);
                    sprintf(actionStr, "%s", lcdStr);
                  }
                }
                sendGLCDcommand(dsDevice, glcd1wCnt, (setColorVGA | setPrintStrXY | setFont),  action2x2[glcd1w[glcd1wCnt].Position].s3Font,
                             0, WHITE,
                             0, 0, 0, actionStr,
                             action2x2[glcd1w[glcd1wCnt].Position].s3X1, action2x2[glcd1w[glcd1wCnt].Position].s3Y1, 0, 0, 0, 1);
                glcd1w[glcd1wCnt].Item += 1;
                break;
              }

              case glcdPrintS2Val:
              {
                if( (glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position] == NULL) | 
                    (glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->thPtr == NULL)
                  )
                {
                  sprintf(switchStr, "%s", "UNUSED");
                }else{
                  if(glcd1w[glcd1wCnt].Action[glcd1w[glcd1wCnt].Position]->thPtr->chipStatus == 'N')
                  {
                    lcdCenterStr((char *)"ON", sizeof(switchStr));
                  }else{
                   lcdCenterStr((char *)"OFF", sizeof(switchStr));
                  }
                  sprintf(switchStr, "%s", lcdStr);
                }

                sendGLCDcommand(dsDevice, glcd1wCnt, (setColorVGA | setPrintStrXY | setFont),  action2x2[glcd1w[glcd1wCnt].Position].s4Font,
                             0, WHITE,
                             0, 0, 0, switchStr,
                             action2x2[glcd1w[glcd1wCnt].Position].s4X1, action2x2[glcd1w[glcd1wCnt].Position].s4Y1, 0, 0, 0, 1);
                glcd1w[glcd1wCnt].Item = 0;
                glcd1w[glcd1wCnt].Position += 1;                
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
            
            switch(glcd1w[glcd1wCnt].Position)
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
                glcd1w[glcd1wCnt].Position = 0;
                sendGLCDcommand(dsDevice, glcd1wCnt, setPageDisplay, 0, 0, 0, glcd1w[glcd1wCnt].Page, 0, 0, nullStr, 0, 0, 0, 0, 0, 1); // Set the page to write into
                glcd1w[glcd1wCnt].Page     += 1;  // set net page to write into
                glcd1wCnt += 1; // on to next display
                break;
              }
            }
            
            switch(glcd1w[glcd1wCnt].Page)
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
                glcd1w[glcd1wCnt].Item     = 0;
                glcd1w[glcd1wCnt].Position = 0;
                glcd1w[glcd1wCnt].Page     = 0;
                glcd1wCnt += 1;
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
        glcd1wCnt += 1;
        if( glcd1wCnt > 7)
        {
          glcd1wCnt = 0;
        }
      }

      break;
    }
    
    case 0:
    default:
    {
      
      glcd1wCnt = 0; // reset the count and start over
      break;
    }
  }
  if(setDebug & glcd1WLED)
    setLED(LED1, ledON);
//    digitalWrite(LED1, OFF);  
}

void writeToGLCD(uint8_t page, uint8_t x)
{
  char str1[displayStrSize+1];

  sendGLCDcommand(dsDevice, x, (setInitL | setPageWrite | setLCDon), 0, 
                   0, 0,
                   page, 0, 0, nullStr,
                   0, 0, 0, 0, 0, 1);

  sprintf(str1, "Display %d", x);
  sendGLCDcommand(dsDevice, x, (setBackVGA | setColorVGA | setFont | setPrintStr ), UBUNTUBOLD, 
                   BLACK,  WHITE,
                   0, 0, 0, str1,
                   0, 0, 0, 0, 0, 1);

  sprintf(str1, "%0X, %0X, %0X, %0X, %0X, %0X, %0X, %0X", 
          glcd1w[x].Addr[0],glcd1w[x].Addr[1],glcd1w[x].Addr[2],glcd1w[x].Addr[3],
          glcd1w[x].Addr[4],glcd1w[x].Addr[5],glcd1w[x].Addr[6],glcd1w[x].Addr[7]);
  sendGLCDcommand(dsDevice, x, (setPrintStr), 0, 
                   0, 0,
                   0, 2, 0, str1,
                   0, 0, 0, 0, 0, 1);

  sprintf(str1, "Page %d - %ld", page, glcdCnt32);
  glcdCnt32++;
  sendGLCDcommand(dsDevice, x, (setPrintStr), 0, 
                   0, 0,
                   0, 4, 0, str1,
                   0, 0, 0, 0, 0, 1);

  sendGLCDcommand(dsDevice, x, (setPageDisplay), 0, 
                   0, 0,
                   page, 0, 0, nullStr,
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

  sendGLCDcommand(dsDevice, x, (setInitL | setPageWrite), 0,
              0, 0,
              0, 0, 0, nullStr,
              0, 0, 0, 0, 0, 1);

  sprintf(str1, "Display %d", cnt);
  sendGLCDcommand(dsDevice, x, (setBackVGA | setColorVGA | setFont | setPrintStr ), UBUNTUBOLD, 
                   BLACK, WHITE,
                   0, 0, 0, str1,
                   0, 0, 0, 0, 0, 1);

  sprintf(str1, "%0X, %0X, %0X, %0X, %0X, %0X, %0X, %0X", 
          glcd1w[cnt].Addr[0],glcd1w[cnt].Addr[1],glcd1w[cnt].Addr[2],glcd1w[cnt].Addr[3],
          glcd1w[cnt].Addr[4],glcd1w[cnt].Addr[5],glcd1w[cnt].Addr[6],glcd1w[cnt].Addr[7]);
  sendGLCDcommand(dsDevice, x, (setPrintStr), 0, 
                   0, 0,
                   0, 2, 0, str1,
                   0, 0, 0, 0, 0, 1);

  sendGLCDcommand(dsDevice, x, (setPageDisplay), 0, 
                   0, 0,
                   0, 0, 0, nullStr,
                   0, 0, 0, 0, 0, 1);

  if(setDebug & glcdSerialDebug)
  {
    myDebug[debugPort]->print(F("GLCD "));
    myDebug[debugPort]->print(cnt);
    myDebug[debugPort]->print(F(" Address 0x"));
    myDebug[debugPort]->print((uint32_t) &(glcd1w[cnt].Addr), HEX);
    myDebug[debugPort]->print(F(" = {"));

    for( int i = 0; i < chipAddrSize; i++)
    {
      if(glcd1w[cnt].Addr[i]>=0 && glcd1w[cnt].Addr[i]<16)
      {
        myDebug[debugPort]->print(F("0x0"));
      }else{
        myDebug[debugPort]->print(F("0x"));
      }
      myDebug[debugPort]->print(glcd1w[cnt].Addr[i], HEX);
      if(i < 7){myDebug[debugPort]->print(F(","));}
    }
    myDebug[debugPort]->println(F("}"));
  }
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
      myDebug[debugPort]->print(F("Resetting GLCD "));
      myDebug[debugPort]->print(q);
      myDebug[debugPort]->println(F(" TeensyNet"));
    }
    sendGLCDcommand(dsDevice, q, setResetDisplay, 0,0,0,0,0,0,nullStr,0,0,0,0,0,1); //reset the GLCD Display
    delay(2000);
    sendGLCDcommand(dsDevice, q, setInitL, 0,0,0,0,0,0,nullStr,0,0,0,0,0,1); //initialize to landscape mode
    for(uint8_t x = 0; x < 8; x++)
    {
      sendGLCDcommand(dsDevice, q, setPageWrite, 0,0,0,x,0,0,nullStr,0,0,0,0,0,1); // select page to write to       
      delay(100);
      sendGLCDcommand(dsDevice, q, clrScn, 0,0,0,0,0,0,nullStr,0,0,0,0,0,1); // clear the page
      delay(100);
      sendGLCDcommand(dsDevice, q, setPageDisplay, 0,0,0,x,0,0,nullStr,0,0,0,0,0,1); // display the page
      delay(100);
      if(setDebug & glcdSerialDebug)
      {
        myDebug[debugPort]->print(F("Initializing GLCD "));
        myDebug[debugPort]->print(q);
        myDebug[debugPort]->print(F(", Page "));
        myDebug[debugPort]->println(x);
      }
      sprintf(displayStr, "GLCD %d, Page %d Initialized", q, x);
      sendGLCDcommand(dsDevice, q, (setPrintStr | setFont | setColorVGA), UBUNTUBOLD,0,WHITE,0,0,0,displayStr,0,0,0,0,0,1); // clear the page
      delay(1000);
    }
  }
}

void clearGLCDdisplayStr(void)
{
  uint8_t x;
  
  for(x = 0; x < displayStrSize; x++)
  {
    displayStr[x] = ' ';
  }
  displayStr[x] = 0x00;
}


/**** End   TeensyNetGLCD functions ****/

