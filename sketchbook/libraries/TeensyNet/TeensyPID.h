/*  TeensyPID.h
    Version 0.01 0831/2014
    by Jim Mayhugh
*/
#ifndef TPID_H
#define TPID_H

// PID Stuff

const uint8_t maxPIDs = 4;
uint8_t pidCnt = 0;

typedef struct
{
  bool       pidEnabled;
  chipStruct *tempPtr;
  double     pidSetPoint;
  chipStruct *switchPtr;
  double     pidKp;
  double     pidKi;
  double     pidKd;
  int        pidDirection;
  uint32_t   pidWindowSize;
  uint32_t   pidwindowStartTime;
  double     pidInput;
  double     pidOutput;
  PID       *myPID;
}chipPIDStruct;

const chipPIDStruct pidClear = { FALSE, NULL, 70, NULL, 0, 0, 0, 0, 5000, 0, 0, 0, NULL };

chipPIDStruct ePID[maxPIDs] =
{
  { FALSE, NULL, 70, NULL, 0, 0, 0, 0, 5000, 0, 0, 0, NULL },
  { FALSE, NULL, 70, NULL, 0, 0, 0, 0, 5000, 0, 0, 0, NULL },
  { FALSE, NULL, 70, NULL, 0, 0, 0, 0, 5000, 0, 0, 0, NULL },
  { FALSE, NULL, 70, NULL, 0, 0, 0, 0, 5000, 0, 0, 0, NULL }
};

//Specify the links and initial tuning parameters
PID PID0(&ePID[0].pidInput,   &ePID[0].pidOutput,  &ePID[0].pidSetPoint,  (double) ePID[0].pidKp,  (double) ePID[0].pidKi,  (double) ePID[0].pidKd,  ePID[0].pidDirection);
PID PID1(&ePID[1].pidInput,   &ePID[1].pidOutput,  &ePID[1].pidSetPoint,  (double) ePID[1].pidKp,  (double) ePID[1].pidKi,  (double) ePID[1].pidKd,  ePID[1].pidDirection);
PID PID2(&ePID[2].pidInput,   &ePID[2].pidOutput,  &ePID[2].pidSetPoint,  (double) ePID[2].pidKp,  (double) ePID[2].pidKi,  (double) ePID[2].pidKd,  ePID[2].pidDirection);
PID PID3(&ePID[3].pidInput,   &ePID[3].pidOutput,  &ePID[3].pidSetPoint,  (double) ePID[3].pidKp,  (double) ePID[3].pidKi,  (double) ePID[3].pidKd,  ePID[3].pidDirection);

PID *pidArrayPtr[] = {&PID0, &PID1, &PID2, &PID3};

// End PID Stuff
#endif
