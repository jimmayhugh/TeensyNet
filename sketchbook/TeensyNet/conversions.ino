/********************

TeensyNet conversions functions

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

void asciiArrayToHexArray(char* result, char* addrDelim, uint8_t* addrVal)
{
  char *addrResult = NULL;
  uint16_t addrResultCnt = 0;
  char resultStr[100];
  uint8_t rsCnt = 0;
  
  for(int x = 0; x < 100; x++)
    resultStr[x] = 0x00;

  if((setDebug & pidDebug) || (setDebug & lcdDebug) || 
     (setDebug & lcdSerialDebug) ||(setDebug & lcdNameUpdate) ||
     (setDebug & chipNameDebug)
    )
    myDebug[debugPort]->println(F("asciiArrayToHexArray Enter"));
      
  addrResult = strtok( result, addrDelim );
  while(addrResult != NULL)
  {
    addrVal[addrResultCnt] = (uint8_t) strtol(addrResult, NULL, 16);
    
  if((setDebug & pidDebug) || (setDebug & lcdDebug) || 
     (setDebug & lcdSerialDebug) ||(setDebug & lcdNameUpdate) ||
     (setDebug & chipNameDebug)
    )
      rsCnt += sprintf(resultStr + rsCnt, "0x%02X,", addrVal[addrResultCnt]);
      
    addrResultCnt++;
    addrResult = strtok( NULL, addrDelim );
  }
   
  if((setDebug & pidDebug) || (setDebug & lcdDebug) || 
     (setDebug & lcdSerialDebug) ||(setDebug & lcdNameUpdate) ||
     (setDebug & chipNameDebug)
    )
  {
    resultStr[rsCnt-1] = 0x00; // remove the last comma
    myDebug[debugPort]->print(F("resultStr = "));
    myDebug[debugPort]->println(resultStr);
    myDebug[debugPort]->println(F("asciiArrayToHexArray Exit"));
  }
}

uint8_t matchAddress(uint8_t* addr, uint8_t* array, uint16_t aSize, uint8_t maxNum)
{
   uint8_t addrCnt, arrayCnt = 0, addrMatch = 0xFF; // 0 = no match, 1 = match
   uint8_t *addrPtr;
   
  if((setDebug & pidDebug) || (setDebug & lcdDebug) || 
     (setDebug & lcdSerialDebug) ||(setDebug & lcdNameUpdate) ||
     (setDebug & chipNameDebug)
    )
  {
    myDebug[debugPort]->println(F("matchAddress Enter"));
    myDebug[debugPort]->print(F("Structure Size = "));
    myDebug[debugPort]->println(aSize);
  }

  while(arrayCnt < maxNum)
  {    
    myDebug[debugPort]->print(F("addr = "));
    for(uint8_t x = 0; x < chipAddrSize; x++)
    {
      myDebug[debugPort]->print(F("0x"));
      if(addr[x] < 0x0F)
        myDebug[debugPort]->print(F("0"));
      myDebug[debugPort]->print(addr[x], HEX);
      if(x < (chipAddrSize - 1))
        myDebug[debugPort]->print(F(","));
    }
    myDebug[debugPort]->println();

    myDebug[debugPort]->print(F("array = "));
    for(uint8_t x = 0; x < chipAddrSize; x++)
    {
      myDebug[debugPort]->print(F("0x"));
      if(array[x] < 0x0F)
        myDebug[debugPort]->print(F("0"));
      myDebug[debugPort]->print(array[x], HEX);
      if(x < (chipAddrSize - 1))
        myDebug[debugPort]->print(F(","));
    }
    myDebug[debugPort]->println();

    for(addrCnt = 0; addrCnt < chipAddrSize; addrCnt++)
    {
      if(array[addrCnt] != addr[addrCnt])
      {
        addrMatch = 0x00;
        break;
      }
    }
    
    if(addrMatch == 0x00)
    {
      array += aSize; // adjust structure address to next structure in the array
      arrayCnt++;
      addrMatch = 0xFF;
      continue;
    }else{
      addrMatch = 0x01;
      break;
    }
  }

  if((setDebug & pidDebug) || (setDebug & lcdDebug) || 
     (setDebug & lcdSerialDebug) ||(setDebug & lcdNameUpdate) ||
     (setDebug & chipNameDebug)
    )
  {
    if(addrMatch == 0x00)
    {
      myDebug[debugPort]->println(F("NO MATCH"));
    }else{
      myDebug[debugPort]->print(F("MATCH - returning "));
      myDebug[debugPort]->println(arrayCnt);
    }
    myDebug[debugPort]->println(F("matchAddress Exit"));
  }
  if(addrMatch == 0x00)
    arrayCnt = 0xFF; // error - no match
    
  return(arrayCnt); 
}

void setLED(uint8_t led, uint8_t state)
{
  if(boardVersion >= 18)
  {
    if(state == ledON)
    {
      digitalWrite(led, HIGH);
    }else{
      digitalWrite(led, LOW);
    }
  }else{
    if(state == ledON)
    {
      digitalWrite(led, LOW);
    }else{
      digitalWrite(led, HIGH);
    }
  }
}
