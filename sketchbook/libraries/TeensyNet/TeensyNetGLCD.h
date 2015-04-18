/*	TeensyNetGLCD.h
		Version 0.02 09/07/2014
		by Jim Mayhugh
*/

#ifndef TNGLCD_H
#define TNGLCD_H

#include "../TeensyNet/Teensy1Wire.h"
#include "../TeensyNet/TeensyAction.h"
// #include "../TeensyNet/TeensyGLCD.h"

const uint8_t  displayStrSize = 33;
const uint8_t  maxGLCDactions = 4;
const uint8_t  maxGLCDchips   = 8;
#if __MK20DX128__
const uint8_t  maxGLCDs       = 4; // Maximum number of 1-wire slave GLCDs
#elif __MK20DX256__
const uint8_t  maxGLCDs       = 12; // Maximum number of 1-wire slave GLCDs
#endif

char displayStr[displayStrSize+1];
uint8_t pageCnt0 = 0, pageCnt1 = 0, /*numGLCDs = 0,*/ numberOfGLCDs = 0;

#if USEVGAONLY ==1
typedef struct
{
  uint8_t   deviceType;
  uint8_t   device; 
  uint32_t  flags;                   // various flags to control setup
  uint8_t   font;                    // Font to use - 0 - No Change, 1 - UbuntuBold, 2 - BigFont, 3 - Small Font, 4 - SevenSegNumFont
  uint8_t   bGR;                     // Background VGA_Color
  uint8_t   cR;                      // Character/foreground RED or VGA_Color value for draw*, fill* and print commands 
  uint8_t   dispP;                   // Display Page 0 - 7 (CPLD only)
  uint8_t   lineP;                   // Line Position 0 - 14
  uint8_t   chrP;                    // Character Position 0 - 32 or  0xFC 0=LEFT 0xFD=RIGHT 0xFE=CENTER
  char      dSTR[displayStrSize+1];  // Character string to print
  uint16_t   x1;                     // rectangle x start or x center of circle
  uint16_t   y1;                     // rectangle y start or y center of circle
  uint16_t   x2;                     // rectangle x end
  uint16_t   y2;                     // rectangle y end
  uint16_t   rad;                    // radius of circle
  uint8_t   bRDY;                    // Buffer ready to process = 1
}glcdCMD;

 glcdCMD glcdCLR = {0, 0, 0, 0, 0, 0, 0, 0, 0, "", 0, 0, 0, 0, 0, 0};

#else
typedef struct
{
  uint32_t  flags;                   // various flags to control setup
  uint8_t   font;                    // Font to use - 0 - No Change, 1 - UbuntuBold, 2 - BigFont, 3 - Small Font, 4 - SevenSegNumFont
  uint8_t   bGR;                     // Background RED value or VGA_Color
  uint8_t   bGG;                     // Background GREEN value
  uint8_t   bGB;                     // Background BLUE value
  uint8_t   cR;                      // Character/foreground RED or VGA_Color value for draw*, fill* and print commands 
  uint8_t   cG;                      // Character/foreground GREEN value for draw*, fill* and print commands
  uint8_t   cB;                      // Character/foreground BLUE value for draw*, fill* and print commands
  uint8_t   dispP;                   // Display Page 0 - 7 (CPLD only)
  uint8_t   lineP;                   // Line Position 0 - 14
  uint8_t   chrP;                    // Character Position 0 - 32 or  0xFC=LEFT 0xFD=RIGHT 0xFE=CENTER
  uint16_t  degR;                    // Degress of Rotation 0-359 Default = 0
  char      dSTR[displayStrSize+1];  // Character string to print
  uint16_t   x1;                     // rectangle x start or x center of circle
  uint16_t   y1;                     // rectangle y start or y center of circle
  uint16_t   x2;                     // rectangle x end
  uint16_t   y2;                     // rectangle y end
  uint16_t   rad;                    // radius of circle
  uint8_t   bRDY;                    // Buffer ready to process = 1
}glcdCMD;

 glcdCMD glcdCLR = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "", 0, 0, 0, 0, 0, 0};

#endif

const uint32_t setInitL           = 0x00000001;               // initialize screen (Landscape)
const uint32_t setInitP           = (setInitL          << 1); // 0x00000002 //initialize screen (Portrait)
const uint32_t clrScn             = (setInitP          << 1); // 0x00000004;  // clear scrn flag
const uint32_t setFillScrRGB      = (clrScn            << 1); // 0x00000008;  // set fillSCR() with RGB values
const uint32_t setFillScrVGA      = (setFillScrRGB     << 1); // 0x00000010;  // set fillSCR() with VGA coloors
const uint32_t setBackRGB         = (setFillScrVGA     << 1); // 0x00000020;  // set setBackColor() with RGB values
const uint32_t setBackVGA         = (setBackRGB        << 1); // 0x00000040;  // set setBackColor() with VGA values
const uint32_t setColorRGB        = (setBackVGA        << 1); // 0x00000080;  // set setColor() with RGB values
const uint32_t setColorVGA        = (setColorRGB       << 1); // 0x00000100;  // set setColor() with VGA values
const uint32_t setFont            = (setColorVGA       << 1); // 0x00000200;  // set font
const uint32_t setDispPage        = (setFont           << 1); // 0x00000400;  // set display Page (CPLD only)
const uint32_t setWrPage          = (setDispPage       << 1); // 0x00000800;  // set Page to Write Page (CPLD only)
const uint32_t setDrawPixel       = (setWrPage         << 1); // 0x00001000;  // set drawPixel
const uint32_t setDrawLine        = (setDrawPixel      << 1); // 0x00002000;  // set drawLine()
const uint32_t setDrawRect        = (setDrawLine       << 1); // 0x00004000;  // set drawRect()
const uint32_t setFillRect        = (setDrawRect       << 1); // 0x00008000;  // set fillRect();
const uint32_t setDrawRRect       = (setFillRect       << 1); // 0x00010000;  // set drawRoundRec()
const uint32_t setDrawFRRect      = (setDrawRRect      << 1); // 0x00020000;  // set drawFillRoundRec()
const uint32_t setDrawCircle      = (setDrawFRRect     << 1); // 0x00040000;  // set drawCircle()
const uint32_t setDrawFillCircle  = (setDrawCircle     << 1); // 0x00080000;  // set fillCircle()
const uint32_t setPrintStr        = (setDrawFillCircle << 1); // 0x00100000;  // set fillCircle()
const uint32_t setPrintStrXY      = (setPrintStr       << 1); // 0x00200000;  // set PrintStr() - print string using X1 and Y1 co-ordinates
const uint32_t setLCDoff          = (setPrintStrXY     << 1); // 0x00400000;  // Turn LCD backlight off (CPLD only)
const uint32_t setLCDon           = (setLCDoff         << 1); // 0x00800000;  // Turn LCD backlight ofn (CPLD only)
const uint32_t setPageWrite       = (setLCDon          << 1); // 0x01000000;  // Set Page to write data into (CPLD only)
const uint32_t setPageDisplay     = (setPageWrite      << 1); // 0x02000000;  // Set Page to display (CPLD only)
const uint32_t setResetDisplay    = 0x80000000;               // reset Teensy31 and display


union glcdData
{
  glcdCMD glcdBUF;
  uint8_t glcdARRAY[sizeof(glcdCMD)];
}glcdUNION;

enum fonts { NONE, UBUNTUBOLD, BIGFONT, SMALLFONT, SEVENSEGMENTNUMFONT, EXTRA };
enum vgaColor {BLACK, WHITE, RED, GREEN, BLUE, SILVER, GRAY, MAROON, YELLOW, OLIVE, LIME, AQUA, TEAL, NAVY, FUCHSIA, PURPLE};

typedef struct
{
  uint16_t rectX1;
  uint16_t rectY1;
  uint16_t rectX2;
  uint16_t rectY2;
  uint8_t  tFont;
  uint16_t tX1;
  uint16_t tY1;
  uint8_t  vFont;
  uint16_t vX1;
  uint16_t vY1;
  uint8_t  s1Font;
  uint16_t s1X1;
  uint16_t s1Y1;
  uint8_t  s2Font;
  uint16_t s2X1;
  uint16_t s2Y1;
  uint8_t  s3Font;
  uint16_t s3X1;
  uint16_t s3Y1;
  uint8_t  s4Font;
  uint16_t s4X1;
  uint16_t s4Y1;
}glcdActionCoOrd;

const glcdActionCoOrd action2x2[4] =
{
  {0,   0, 385, 230,
   UBUNTUBOLD,      10,       10,
   EXTRA,      action2x2[0].tX1+80, action2x2[0].tY1+32,
   UBUNTUBOLD, action2x2[0].tX1,    action2x2[0].vY1+55,
   UBUNTUBOLD, action2x2[0].vX1+20, action2x2[0].s1Y1+32,
   UBUNTUBOLD, action2x2[0].tX1,    action2x2[0].s2Y1+35,
   UBUNTUBOLD, action2x2[0].s2X1,   action2x2[0].s3Y1+32
  },
  {390,   0, 795, 230,
   UBUNTUBOLD, 410,  10,
   EXTRA,      action2x2[1].tX1+80,  action2x2[1].tY1+32,
   UBUNTUBOLD, action2x2[1].tX1,     action2x2[1].vY1+55,
   UBUNTUBOLD, action2x2[1].vX1+20,  action2x2[1].s1Y1+32,
   UBUNTUBOLD, action2x2[1].tX1,     action2x2[1].s2Y1+35,
   UBUNTUBOLD, action2x2[1].s2X1,    action2x2[1].s3Y1+32
  },
  {  0, 240, 385, 475,
   UBUNTUBOLD,  10, 245,
   EXTRA,      action2x2[2].tX1+80, action2x2[2].tY1+32,
   UBUNTUBOLD, action2x2[2].tX1,    action2x2[2].vY1+55,
   UBUNTUBOLD, action2x2[2].vX1+20, action2x2[2].s1Y1+32,
   UBUNTUBOLD, action2x2[2].tX1,    action2x2[2].s2Y1+35,
   UBUNTUBOLD, action2x2[2].s2X1,   action2x2[2].s3Y1+32
  },
  {390, 240, 795, 475,
   UBUNTUBOLD, 410, 245,
   EXTRA,      action2x2[3].tX1+80, action2x2[3].tY1+32,
   UBUNTUBOLD, action2x2[3].tX1,    action2x2[3].vY1+55,
   UBUNTUBOLD, action2x2[3].vX1+20, action2x2[3].s1Y1+32,
   UBUNTUBOLD, action2x2[3].tX1,    action2x2[3].s2Y1+35,
   UBUNTUBOLD, action2x2[3].s2X1,   action2x2[3].s3Y1+32
  }
};

typedef struct
{
  uint16_t rectX1;
  uint16_t rectY1;
  uint16_t rectX2;
  uint16_t rectY2;
  uint8_t   tFont;
  uint16_t tX1;
  uint16_t tY1;
  uint8_t   vFont;
  uint16_t vX1;
  uint16_t vY1;
}glcdChipCoOrd;

const glcdChipCoOrd chip2x4[8] =
{
  {  0,   0, 380, 115, UBUNTUBOLD,  10,   5, EXTRA, 110,  45},
  {385,   0, 795, 115, UBUNTUBOLD, 410,   5, EXTRA, 510,  45},
  {  0, 120, 380, 230, UBUNTUBOLD,  10, 125, EXTRA, 110, 165},
  {385, 120, 795, 230, UBUNTUBOLD, 410, 125, EXTRA, 510, 165},
  {  0, 235, 380, 345, UBUNTUBOLD,  10, 240, EXTRA, 110, 280},
  {385, 235, 795, 345, UBUNTUBOLD, 410, 240, EXTRA, 510, 280},
  {  0, 350, 380, 460, UBUNTUBOLD,  10, 355, EXTRA, 110, 395},
  {385, 350, 795, 460, UBUNTUBOLD, 410, 355, EXTRA, 510, 395}
};

uint8_t glcdCnt = 0, glcd1wCnt = 0;
uint32_t glcdCnt32 = 0;
const uint8_t glcdFActive = 0x01;
const uint8_t glcdFAction = (glcdFActive << 1);
const uint8_t glcdFChip   = (glcdFAction << 1);

const uint8_t glcdActionSetPage = 0;
const uint8_t glcdDrawRectangle = glcdActionSetPage + 1;
const uint8_t glcdPrintTempName = glcdDrawRectangle + 1;
const uint8_t glcdPrintTemp     = glcdPrintTempName + 1;
const uint8_t glcdPrintS1Name   = glcdPrintTemp     + 1;
const uint8_t glcdPrintS1Val    = glcdPrintS1Name   + 1;
const uint8_t glcdPrintS2Name   = glcdPrintS1Val    + 1;
const uint8_t glcdPrintS2Val    = glcdPrintS2Name   + 1;

typedef struct
{
  uint8_t                 Addr[chipAddrSize];
  char                    Name[displayStrSize+1];
  uint8_t                 Flags;
  uint8_t                 Item;
  uint8_t                 Position;
  uint8_t                 Page;
  chipActionStruct        *Action[maxGLCDactions];
  chipStruct              *Chip[maxGLCDchips];
}glcd1wStruct;


glcd1wStruct glcd1w[maxGLCDs] = 
{
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
#if __MK20DX256__
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
/*
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
*/
#endif
  { {0,0,0,0,0,0,0,0},  "", 0, 0, 0, 0, { NULL, NULL, NULL, NULL }, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } }
};
#endif
