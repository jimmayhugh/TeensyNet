/********************

I2CEEPROMAnything.h

Version 0.0.2
Last Modified 04/05/2015
By Jim Mayhugh

Based on code from http://playground.arduino.cc/Code/EEPROMWriteAnything

Use external EEPROM (24LC512) for structure storage, and uses Teensy 3.X board.

 ****NOTE****
 This code has been tested on the Teensy 3.x boards from http://www.pjrc.com .
 In order to take full advantage of the capabilities of the Teensy 3.x, the following
 must be placed in the Teensyduino Wire.h file:
 
 #if defined(__MK20DX128__) || defined(__MK20DX256__)
  #define BUFFER_LENGTH 130
 #else
  #define BUFFER_LENGTH 32
 #endif
 
 replacing
  #define BUFFER_LENGTH 32
 ****NOTE****
 
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

#include <WProgram.h>

template <class T> uint16_t I2CEEPROM_writeAnything(uint16_t ee, const T& value, uint8_t device)
{
  const byte* p = (const byte*)(const void*)&value;
  int32_t i, totalSent = 0, leftToSend = 0, sendNext;
  const uint8_t maxBufferSize = 128; // on-chip page buffer
//  uint8_t sent = 0;
  leftToSend = sizeof(value);
/*
  Serial.print(F("leftToSend ="));
  Serial.println(leftToSend);
*/
  do
  {
    if(leftToSend >= maxBufferSize)
    {
      sendNext = maxBufferSize;
    }else{
      sendNext = leftToSend;
    }
    Wire.beginTransmission(device);
    Wire.send((int)(ee >> 8)); // MSB
    Wire.send((int)(ee & 0xFF)); // LSB
    for (i = 0; i < sendNext; i++)
    {
      Wire.write(*p++);
    }
    Wire.endTransmission();
    delay(50);
    leftToSend = leftToSend - sendNext;
    ee = ee + maxBufferSize;
    totalSent = totalSent + sendNext;
  }while(leftToSend > 0);
  Wire.endTransmission();
  delay(50);
  return totalSent;
}

template <class T> uint16_t I2CEEPROM_readAnything(uint16_t ee, T& value, uint8_t device)
{
  byte* p = (byte*)(void*)&value;
  uint32_t i, totalRead = 0, leftToRead = 0, readNext;
  const uint8_t maxBufferSize = 128;
//  uint8_t received;
  leftToRead = sizeof(value);
/*
  Serial.print(F("leftToRead ="));
  Serial.println(leftToRead);
*/
  do
  {
    if(leftToRead >= maxBufferSize)
    {
      readNext = maxBufferSize;
    }else{
      readNext = leftToRead;
    }
    Wire.beginTransmission(device);
    Wire.send((int)(ee >> 8)); // MSB
    Wire.send((int)(ee & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(device, (size_t) readNext);
    for (i = 0; i < readNext; i++)
    {
      if(Wire.available())
      {
        *p++ = Wire.read();
        
      }
    }
    leftToRead = leftToRead - readNext;
    ee = ee + maxBufferSize;
    totalRead = totalRead + readNext;
   }while(leftToRead > 0);
  return totalRead;
}

