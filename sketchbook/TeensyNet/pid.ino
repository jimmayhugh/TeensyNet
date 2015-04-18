/********************

TeensyNet pid functions

Version 0.0.49
Last Modified 04/04/2015
By Jim Mayhugh

Uses the 24LC512 EEPROM for structure storage, and Teensy 3.1 board

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


void pidSetup(void)
{
/// ***** Start PID Setup *****

  for(int x =0; x < maxPIDs; x++)
  {
    ePID[x].myPID = pidArrayPtr[x];
  //tell the PID to range between 0 and the full window size
    ePID[x].myPID->SetOutputLimits(0, ePID[x].pidWindowSize);
    
    ePID[x].myPID->SetTunings(ePID[x].pidKp, ePID[x].pidKi, ePID[x].pidKd);
    
    ePID[x].myPID->SetSampleTime(200);

    if(ePID[x].pidDirection == 1)
    {
      ePID[x].myPID->SetControllerDirection(DIRECT);
    }else{
      ePID[x].myPID->SetControllerDirection(REVERSE);
    }
    
    
  //turn the PID on if variable are non-zero
    if( (ePID[x].pidEnabled == TRUE) &&
        (ePID[x].pidKp != 0) &&
        (ePID[x].pidKi != 0) &&
        (ePID[x].pidKd != 0) &&
        (ePID[x].pidWindowSize != 0)
        )
    {
      ePID[x].myPID->SetMode(AUTOMATIC);
    }else{
      ePID[x].pidEnabled = FALSE;
      ePID[x].myPID->SetMode(MANUAL);
    }
  }

}



void updatePIDs(uint8_t pidCnt)
{
  
  if(ePID[pidCnt].pidEnabled == 1)
  {    
    // *** Start PID Loop ***
    ePID[pidCnt].pidInput = (double) ePID[pidCnt].tempPtr->chipStatus;

    if(setDebug & pidDebug)
    {
      myDebug[debugPort]->println(F("Entering updatePIDs"));
      myDebug[debugPort]->print(F("PID #"));
      myDebug[debugPort]->println(pidCnt);
      myDebug[debugPort]->print(F("ePID["));
      myDebug[debugPort]->print(pidCnt);
      myDebug[debugPort]->print(F("].pidInput = "));
      myDebug[debugPort]->println((double) ePID[pidCnt].pidInput);
      myDebug[debugPort]->print(F("ePID["));
      myDebug[debugPort]->print(pidCnt);
      myDebug[debugPort]->print(F("].pidKp = "));
      myDebug[debugPort]->println(ePID[pidCnt].pidKp);
      myDebug[debugPort]->print(F("ePID["));
      myDebug[debugPort]->print(pidCnt);
      myDebug[debugPort]->print(F("].pidKi = "));
      myDebug[debugPort]->println(ePID[pidCnt].pidKi);
      myDebug[debugPort]->print(F("ePID["));
      myDebug[debugPort]->print(pidCnt);
      myDebug[debugPort]->print(F("].pidKd = "));
      myDebug[debugPort]->println(ePID[pidCnt].pidKd);
      myDebug[debugPort]->print(F("ePID["));
      myDebug[debugPort]->print(pidCnt);
      myDebug[debugPort]->print(F("].pidDirection = "));
      myDebug[debugPort]->println(ePID[pidCnt].pidDirection);
      myDebug[debugPort]->print(F("ePID["));
      myDebug[debugPort]->print(pidCnt);
      myDebug[debugPort]->print(F("].pidWindowStartTime = "));
      myDebug[debugPort]->println((uint32_t) ePID[pidCnt].pidwindowStartTime);
      myDebug[debugPort]->print(F("millis() = "));
      myDebug[debugPort]->println((uint32_t) millis());
    }
  
    if(ePID[pidCnt].myPID->Compute())
    {
      if(setDebug & pidDebug)
      {
        myDebug[debugPort]->println(F("Compute() returned TRUE"));
      }
    }else{
      if(setDebug & pidDebug)
      {
        myDebug[debugPort]->println(F("Compute() returned FALSE"));
      }
    }

    uint32_t now = millis();
    
    if(setDebug & pidDebug)
    {
      myDebug[debugPort]->print(F("now - ePID[pidCnt].pidwindowStartTime = "));
      myDebug[debugPort]->println(now - ePID[pidCnt].pidwindowStartTime);
    }

  /************************************************
   * turn the output pin on/off based on pid output
   ************************************************/
    if(now - ePID[pidCnt].pidwindowStartTime > ePID[pidCnt].pidWindowSize)
    { //time to shift the Relay Window
      ePID[pidCnt].pidwindowStartTime += ePID[pidCnt].pidWindowSize;
    }
  
      if(setDebug & pidDebug)
      {
        myDebug[debugPort]->print(F("ePID["));
        myDebug[debugPort]->print(pidCnt);
        myDebug[debugPort]->print(F("].pidOutPut = "));
        myDebug[debugPort]->println((double) ePID[pidCnt].pidOutput);
        myDebug[debugPort]->print(F("now = "));
        myDebug[debugPort]->println(now);
        myDebug[debugPort]->print(F("ePID["));
        myDebug[debugPort]->print(pidCnt);
        myDebug[debugPort]->print(F("].pidwindowStartTime = "));
        myDebug[debugPort]->println((double) ePID[pidCnt].pidwindowStartTime);
        myDebug[debugPort]->print(F("now - ePID["));
        myDebug[debugPort]->print(pidCnt);
        myDebug[debugPort]->print(F("].pidwindowStartTime = "));
        myDebug[debugPort]->println((double) now - ePID[pidCnt].pidwindowStartTime);
  
        myDebug[debugPort]->print((double) ePID[pidCnt].pidOutput);
        
        if(ePID[pidCnt].pidOutput > now - ePID[pidCnt].pidwindowStartTime)
        {
          myDebug[debugPort]->print(F(" > "));
        }else{
          myDebug[debugPort]->print(F(" < "));
        }
        myDebug[debugPort]->println((double) now - ePID[pidCnt].pidwindowStartTime);
      }

    if(ePID[pidCnt].pidOutput > now - ePID[pidCnt].pidwindowStartTime)
    {
      if(setDebug & pidDebug)
      {
        myDebug[debugPort]->println(F("Turning Switch ON"));
      }
      actionSwitchSet((uint8_t *) &ePID[pidCnt].switchPtr->chipAddr, ds2406PIOAon);
    }else{
      if(setDebug & pidDebug)
      {
        myDebug[debugPort]->println(F("Turning Switch OFF"));
      }
      actionSwitchSet((uint8_t *) &ePID[pidCnt].switchPtr->chipAddr, ds2406PIOAoff);
    }
  // *** End PID Loop ***

    if(setDebug & pidDebug)
      {
        myDebug[debugPort]->print(F("ePID["));
        myDebug[debugPort]->print(pidCnt);
        myDebug[debugPort]->print(F("].pidOutput = "));
        myDebug[debugPort]->println((double) ePID[pidCnt].pidOutput);
        myDebug[debugPort]->println(F("Exiting updatePIDs"));
      }

  }else{
    ePID[pidCnt].myPID->SetMode(MANUAL);
  }
}

void getPIDStatus(uint8_t x)
{ 
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d", (int) ePID[x].pidEnabled);
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", ' ');
    if(ePID[x].tempPtr == NULL)
    {
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", charChipAddrArray);      
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c" ,' ');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", unassignedStr);      
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c" ,' ');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",(int) noChipPresent);
    }else{
      showChipAddress((uint8_t *) ePID[x].tempPtr->chipAddr);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c" ,' ');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", ePID[x].tempPtr->chipName);      
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c" ,' ');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",(int) ePID[x].tempPtr->chipStatus);
    }
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", ' ');
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",(int) ePID[x].pidSetPoint);
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", ' ');
    if(ePID[x].switchPtr == NULL)
    {
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", charChipAddrArray);      
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c" ,' ');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", unassignedStr);      
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c" ,' ');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",(int) noChipPresent);
    }else{
      showChipAddress((uint8_t *) ePID[x].switchPtr->chipAddr);
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c" ,' ');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%s", ePID[x].switchPtr->chipName);      
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c" ,' ');
      rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c",(char) ePID[x].switchPtr->chipStatus);
    }
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", ' ');
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%.2f",(double) ePID[x].pidKp);
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", ' ');
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%.2f",(double) ePID[x].pidKi);
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", ' ');
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%.2f",(double) ePID[x].pidKd);
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", ' ');
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%d",(int) ePID[x].pidDirection);
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%c", ' ');
    rBuffCnt += sprintf(ReplyBuffer+rBuffCnt, "%ld",(uint32_t) ePID[x].pidWindowSize);
}


