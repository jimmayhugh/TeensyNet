/********************

TeensyNet ds2762 functions

Version 0.0.49
Last Modified 04/04/2015
By Jim Mayhugh

The MAXIM ds2762 is used to measure the micro voltages of a k-type thermocouple
and using a table-driven approach, derive a temperature.

This device has been supplanted by the MAXIM MAX31850 1-wire k-type thermocouple reader,
but is kept for historical purposes.

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

void Read_TC_Volts(uint8_t x)
{ 
  if(setDebug & ds2762Debug)
  {
    myDebug[debugPort]->println(F("Enter Read_TC_Volts"));
  }
  ds.reset();
  ds.select(chip[x].chipAddr);
  ds.write(0x69); //read voltage
  ds.write(0x0e);
  for (i = 0; i < 2; i++)
  {
    voltage[i] = ds.read();
    if(setDebug & ds2762Debug)
    {
      myDebug[debugPort]->print(F("voltage["));
      myDebug[debugPort]->print(i);
      myDebug[debugPort]->print(F("] = 0x"));
      if(voltage[i] < 0x10){myDebug[debugPort]->print(F("0"));}
      myDebug[debugPort]->print(voltage[i], HEX);
      myDebug[debugPort]->print(F(" "));
    }
  }
  if(setDebug & ds2762Debug)
  {
    myDebug[debugPort]->println();
  }
  ds.reset();
  tcVoltage = (voltage[0] << 8) + voltage[1];
  tcVoltage >>= 3; 
  if((voltage[0] & 0x80) == 0x80)
  {
    sign = 1;
    tcVoltage |= 0xF000;
    tcVoltage = ~tcVoltage;
    tcVoltage += 1;
  }else{
    sign = 0;
  }
  tcBuff = tcVoltage * 15;
  tcVoltage *= 5;
  tcVoltage >>= 3;
  tcVoltage += tcBuff;
  
  if(setDebug & ds2762Debug)
  {
    myDebug[debugPort]->print(F("tcVoltage = "));
    myDebug[debugPort]->print(tcVoltage);
    myDebug[debugPort]->println(F(" microvolts"));
    myDebug[debugPort]->println(F("Exit Read_TC_Volts"));
  }
} 

/* Reads cold junction (device) temperature 
-- each raw bit = 0.125 degrees C 
-- returns tmpCJ in whole degrees C */ 
void Read_CJ_Temp(uint8_t x)
{ 
  if(setDebug & ds2762Debug)
  {
    myDebug[debugPort]->println(F("Enter Read_CJ_Temp"));
  }
  ds.reset();
  ds.select(chip[x].chipAddr);
  ds.write(0x69);
  ds.write(0x18); //read cjTemp
  for (i = 0; i < 2; i++)
  {
    cjTemp[i] = ds.read();
    if(setDebug & ds2762Debug)
    {
      myDebug[debugPort]->print(F("cjTemp["));
      myDebug[debugPort]->print(i);
      myDebug[debugPort]->print(F("] = 0x"));
      if(cjTemp[i] < 0x10){myDebug[debugPort]->print(F("0"));}
      myDebug[debugPort]->print(cjTemp[i], HEX);
      myDebug[debugPort]->print(F(" "));
    }
  }
  if(setDebug & ds2762Debug)
  {
    myDebug[debugPort]->println();
  }
  ds.reset();
  cjTemperature = (cjTemp[0] << 8) + cjTemp[1];
  if(cjTemperature>=0x8000)
  { 
    cjTemperature = 0;
//    cjdTemperature = 0.0; // disallow negative 
  }else{
//    cjdTemperature =  (double) ((double) cjTemperature) * .125;
    cjTemperature >>= 8;
  } 
  if(setDebug & ds2762Debug)
  {
    myDebug[debugPort]->print(F("cjTemperature = "));
    myDebug[debugPort]->print(cjTemperature);
    myDebug[debugPort]->print(F(" degrees C, "));
    myDebug[debugPort]->print(((cjTemperature * 9) / 5) + 32);
    myDebug[debugPort]->println(F(" degrees F")); 
    myDebug[debugPort]->println(F("Exit Read_CJ_Temp"));
  }
} 

/* Search currently selected TC table for nearest entry 
-- uses modified binary algorithm to find cjComp 
-- high end of search set before calling (tblHi) 
-- successful search sets tempC */ 
void TC_Lookup(void)
{ 
  if(setDebug & ds2762Debug)
  {
    myDebug[debugPort]->println(F("Enter TC_Lookup"));
  }
  tblLo=0; // low entry of table 
  tempC=22; // default to room temp
  testVal=pgm_read_word_near(kTable + tblHi); // check max temp
  if(cjComp>testVal)
  { 
    error=1; // out of range 
  }else{ 
    while(1)
    { 
      eePntr=(tblLo+tblHi)/2; // midpoint of search span 
      testVal=pgm_read_word_near(kTable + eePntr); // read value from midpoint
      if(setDebug & ds2762Debug)
      {
        myDebug[debugPort]->print(F("testVal = "));
        myDebug[debugPort]->print(testVal);
      }
      if(cjComp == testVal)
      {
        if(setDebug & ds2762Debug)
        {
          myDebug[debugPort]->println(F(" - TC_Lookup Temp Match"));
        }
//        tempC = eePntr;
        return; // found it! 
      }else{
        if(cjComp<testVal)
        {
          if(setDebug & ds2762Debug)
          {
             myDebug[debugPort]->println(F(" - testVal too BIG"));
          }
         tblHi=eePntr; //search lower half
        }else{
          if(setDebug & ds2762Debug)
          {
             myDebug[debugPort]->println(F(" - testVal too small"));
          }
         tblLo=eePntr; // search upper half
        }
      }
      if(setDebug & ds2762Debug)
      {
        myDebug[debugPort]->print(F("tblHi = "));
        myDebug[debugPort]->print(tblHi);
        myDebug[debugPort]->print(F(", tblLo = "));
        myDebug[debugPort]->println(tblLo);
      }
      if((tblHi-tblLo)<2)
      { // span at minimum 
        if(setDebug & ds2762Debug)
        {
          myDebug[debugPort]->println(F("TC_Lookup Temp Span At Minimum"));
        }
        eePntr=tblLo; 
        return; 
      } 
    } 
  }
}


