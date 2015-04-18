/*  TeensyAction.h
    Version 0.01 0831/2014
    by Jim Mayhugh
*/

#ifndef TA_H
#define TA_H

#if __MK20DX128__
const uint8_t maxActions     = 4; // Maximum number of Actions
#elif __MK20DX256__
const uint8_t maxActions     = 12; // Maximum number of Actions
#endif

typedef struct
{
  bool       actionEnabled;
  chipStruct *tempPtr;
  int16_t    tooCold;
  chipStruct *tcPtr;
  uint8_t    tcSwitchLastState;
  uint32_t   tcDelay;
  uint32_t   tcMillis;
  int16_t    tooHot;
  chipStruct *thPtr;
  uint8_t    thSwitchLastState;
  uint32_t   thDelay;
  uint32_t   thMillis;
  uint8_t    lcdAddr;
  uint32_t   lcdMillis;
}chipActionStruct;

const chipActionStruct actionClear = { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 };

const uint32_t lcdUpdateTimer = 1000;

chipActionStruct action[maxActions] =
{
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
#if __MK20DX256__
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
#endif
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 },
  { FALSE, NULL, -255, NULL, 'F', 0, 0, 255, NULL, 'F', 0, 0, 0, 0 }
};

uint8_t chipBuffer[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t chipCnt, chipX = 0, actionsCnt = 0;

#endif
