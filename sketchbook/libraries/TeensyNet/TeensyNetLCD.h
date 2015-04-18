/*	TeensyNetLCD.h
		Version 0.01 11/22/2014
		by Jim Mayhugh

A 1-wire slave device using the Teensy3.x and an HD44870 LCD controller

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


*/

#ifndef TNLCD_H
#define TNLCD_H

#include "../TeensyNet/Teensy1Wire.h"
#include "../TeensyNet/TeensyAction.h"

const uint8_t lcdCols = 20, lcd1wTempLoc = 15, lcd1SwitchLoc = 17;
uint8_t lcdCnt, num1wLCDs = 0, numberOf1wLCDs = 0;

#if __MK20DX128__
const uint8_t  maxLCDs       = 4; // Maximum number of 1-wire slave LCDs
#elif __MK20DX256__
const uint8_t  maxLCDs       = 12; // Maximum number of 1-wire slave LCDs
#endif

typedef struct
{
  uint8_t   t3LCDcmd;
  uint8_t   row;
  uint8_t   col;
  char      lcdStr[lcdCols+1];
  uint8_t   t3LCDrdy;
}t3LCDcommand;

t3LCDcommand t3LCDclr = {0, 0, 0, "", 0 };

union tc3LCDdata
{
  t3LCDcommand t3LCDbuf;
  uint8_t      lcdArray[sizeof(t3LCDcommand)];
}t3LCDunion;

const uint8_t degreeChar = 0xDF;

const uint8_t clr1wLCD      = 0x01;
const uint8_t set1wLCDon    = (clr1wLCD      << 1); // 0x02
const uint8_t set1wLCDoff   = (set1wLCDon    << 1); // 0x04
const uint8_t set1wLCDBLon  = (set1wLCDoff   << 1); // 0x08
const uint8_t set1wLCDBLoff = (set1wLCDBLon  << 1); // 0x10
const uint8_t prt1wLCD      = (set1wLCDBLoff << 1); // 0x20

const uint8_t lcdFActive = 0x01;
const uint8_t lcdFAction = (lcdFActive << 1);
const uint8_t lcdFChip   = (lcdFAction << 1);

/*****
****** Note: The upper four bits of the structure t3LCDStruct.Flags are used as a position counter
****** in the lcd1wUpdate() function, and are not to be used for any other purpose
*****/

typedef struct
{
  uint8_t                 Addr[chipAddrSize];
  char                    Name[lcdCols+1];
  uint8_t                 Flags;
  chipActionStruct        *Action;
  chipStruct              *Chip[lcdRows];
}t3LCDStruct;

t3LCDStruct lcd1w[maxActions] = 
{
  { {0,0,0,0,0,0,0,0},  "", 0, NULL, {NULL, NULL , NULL , NULL} },
  { {0,0,0,0,0,0,0,0},  "", 0, NULL, {NULL, NULL , NULL , NULL} },
  { {0,0,0,0,0,0,0,0},  "", 0, NULL, {NULL, NULL , NULL , NULL} },
#if __MK20DX256__
  { {0,0,0,0,0,0,0,0},  "", 0, NULL, {NULL, NULL , NULL , NULL} },
  { {0,0,0,0,0,0,0,0},  "", 0, NULL, {NULL, NULL , NULL , NULL} },
  { {0,0,0,0,0,0,0,0},  "", 0, NULL, {NULL, NULL , NULL , NULL} },
  { {0,0,0,0,0,0,0,0},  "", 0, NULL, {NULL, NULL , NULL , NULL} },
  { {0,0,0,0,0,0,0,0},  "", 0, NULL, {NULL, NULL , NULL , NULL} },
  { {0,0,0,0,0,0,0,0},  "", 0, NULL, {NULL, NULL , NULL , NULL} },
  { {0,0,0,0,0,0,0,0},  "", 0, NULL, {NULL, NULL , NULL , NULL} },
  { {0,0,0,0,0,0,0,0},  "", 0, NULL, {NULL, NULL , NULL , NULL} },
#endif
  { {0,0,0,0,0,0,0,0},  "", 0, NULL, {NULL, NULL , NULL , NULL} }
};


#endif


