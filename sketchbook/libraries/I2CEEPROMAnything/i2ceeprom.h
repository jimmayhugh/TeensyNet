#include <WProgram.h>
// #define DEVICE 0x50 //this is the device ID from the datasheet of the 24LC256

//in the normal write anything the eeaddress is incrimented after the writing of each byte. The Wire library does this behind the scenes.

template <class T> int eeWrite(int ee, const T& value, int device)
{
  const byte* p = (const byte*)(const void*)&value;
  uint32_t i;
  Wire.beginTransmission(device);
  Wire.send((int)(ee >> 8)); // MSB
  Wire.send((int)(ee & 0xFF)); // LSB
  for (i = 0; i < sizeof(value); i++)
    Wire.send(*p++);
  Wire.endTransmission();
  return i;
}

template <class T> int eeRead(int ee, T& value, int device)
{
  byte* p = (byte*)(void*)&value;
  uint32_t i;
  Wire.beginTransmission(device);
  Wire.send((int)(ee >> 8)); // MSB
  Wire.send((int)(ee & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(DEVICE,sizeof(value));
  for (i = 0; i < sizeof(value); i++)
    if(Wire.available())
      *p++ = Wire.receive();
  return i;
}

