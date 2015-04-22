// --------------------------------------
// i2c_scanner
//
// Version 1
//    This program (or code that looks like it)
//    can be found in many places.
//    For example on the Arduino.cc forum.
//    The original author is not know.
// Version 2, Juni 2012, Using Arduino 1.0.1
//     Adapted to be as simple as possible by Arduino.cc user Krodal
// Version 3, Feb 26  2013
//    V3 by louarnold
// Version 4, March 3, 2013, Using Arduino 1.0.3
//    by Arduino.cc user Krodal.
//    Changes by louarnold removed.
//    Scanning addresses changed from 0...127 to 1...119,
//    according to the i2c scanner by Nick Gammon
//    http://www.gammon.com.au/forum/?id=10896
// Version 5, March 28, 2013
//    As version 4, but address scans now to 127.
//    A sensor seems to use address 120.
//
// Version 6, April 22, 2015
//    by Jim Mayhugh
//    Allows use of Serial2 for debug
//    If a Teensy3.1, check both I2C channels 
//
// This sketch tests the standard 7-bit addresses
// Devices with higher bit address might not be seen properly.
//

// #define USESERIAL2 1 // if set to 1, use Serial2 with FTDI for debugging

//#include <FastWire.h>
#include <i2c_t3.h>
#include <TeensyTestPoints.h>

uint32_t cnt;
uint32_t baudRate = 115200;
uint8_t x;

void setup()
{
  for(x = LED1; x <= LED5; x++)
  {
    pinMode(x,OUTPUT);       // test LEDs on TeensyNet > V12.0
    digitalWrite(x, HIGH);   // turn them all off
  }
  Wire.begin();
#if __MK20DX256__
  Wire1.begin();
#endif
#if USESERIAL2 == 1
  // reassign pins 26 and 31 to use the ALT3 configuration
  // which makes them  Serial port 2 Rx(26) and Tx(31)
  CORE_PIN26_CONFIG = PORT_PCR_MUX(3);
  CORE_PIN31_CONFIG = PORT_PCR_MUX(3);
  Serial2.begin(baudRate);
  Stream &myDebug = Serial2;
#else
  Serial.begin(baudRate);
  Stream &myDebug = Serial;
#endif
  myDebug.println("\nI2C Scanner");
  cnt = 0;
}

#if USESERIAL2 == 1
  Stream &myDebug = Serial2;
#else
  Stream &myDebug = Serial;
#endif

void loop()
{
  byte error, error1, address;
  int nDevices, n1Devices;

  myDebug.print(F("Count #"));
  myDebug.print(cnt);
  myDebug.println(F(" - Scanning..."));

  nDevices = 0;
  n1Devices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmission to see if
    // a device did acknowledge to the address.
    
    //Do Wire Devices
    digitalWrite(LED5, LOW);   // turn LED5 on
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    digitalWrite(LED5, HIGH);   // turn LED5 off

    if (error == 0)
    {
      myDebug.print("I2C device found at Wire address 0x");
      if (address<16) 
        myDebug.print("0");
      myDebug.print(address,HEX);
      myDebug.println("  !");

      nDevices++;
    }
    else if (error==4) 
    {
      myDebug.print("Unknown error on Wire at address 0x");
      if (address<16) 
        myDebug.print("0");
      myDebug.println(address,HEX);
    }
    
    // Do Wire1 Devices
#if __MK20DX256__
    digitalWrite(LED4, LOW);   // turn LED4 on
    Wire1.beginTransmission(address);
    error1 = Wire1.endTransmission();
    digitalWrite(LED4, HIGH);   // turn LED4 off

    if (error1 == 0)
    {
      myDebug.print("I2C device found at Wire1 address 0x");
      if (address<16) 
        myDebug.print("0");
      myDebug.print(address,HEX);
      myDebug.println("  !");

      n1Devices++;
    }
    else if (error1==4) 
    {
      myDebug.print("Unknown error on Wire1 at address 0x");
      if (address<16) 
        myDebug.print("0");
      myDebug.println(address,HEX);
    }    
#endif

  }

  if (nDevices == 0)
  {
    myDebug.println("No Wire I2C devices found\n");
  }
  
#if __MK20DX256__
  if(n1Devices == 0)
  {
    myDebug.println("No Wire1 I2C devices found\n");
  }
#endif

  myDebug.println("done\n");
  cnt++;
  delay(1000);           // wait 1 second for next scan
}
