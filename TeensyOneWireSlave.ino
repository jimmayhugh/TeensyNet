#define dsslavepin 2


#include <OneWireSlave.h>
#include <MAX31855.h>

//                     {Family , <---, ----, ----, ID--, ----, --->,  CRC} 
unsigned char rom[8] = {0xAA, 0x02, 0x00, 0x02, 0x00, 0x00, 0x02, 0x00};
//                            {TLSB, TMSB, THRH, TLRL, Conf, 0xFF, Rese, 0x10,  CRC}
unsigned char scratchpad[9] = {0x00, 0x00, 0x4B, 0x46, 0x7F, 0xFF, 0x00, 0x10, 0x00};
//                             {TLSB, TMSB}
unsigned char temperature[2] = {0x7F, 0x09};

const int mxCLK = 3;
const int mxDO  = 5;
const int mxCS0 = 4;

MAX31855 eMX0(mxCLK, mxCS0, mxDO);
OneWireSlave ds(dsslavepin);

//Blinking
const int ledPin = 13;
int ledState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated
long interval = 250;            // interval at which to blink (milliseconds)

float value, oldvalue;
boolean expose = true;
int16_t degree = 0;

void setup() {
  attachInterrupt(dsslaveassignedint, slave, CHANGE);
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("Serial started"));
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for Leonardo only
//  }
  pinMode(ledPin, OUTPUT);
  ds.init(rom);
  ds.setScratchpad(scratchpad);
  ds.setPower(EXTERNAL);
  ds.setResolution(9);
  value = -55;
  Serial.println(F("Setting attach44 to temper"));
  ds.attach44h(temper);
  Serial.println(F("Exiting attach44"));
}

void slave() {
  ds.MasterResetPulseDetection();
}

void loop() {
  blinking();
  
//  readfromanalog();
}

void temper() {
  int16_t degree = eMX0.readCelsius();
//  Serial.print(F("temp = 0x"));
//  Serial.println(degree);
//  Serial.print(F("degree = 0x"));
//  Serial.println(degree, HEX);
  temperature[0] = degree & 0xFF;
  temperature[1] = degree >> 8;
  ds.setTemperature(temperature);
  degree++;
}

void blinking() {
  unsigned long currentMillis = millis(); 
  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
  }
}

