#include <OneWire.h>

// Find OneWire devices
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

OneWire  ds(2);  // on pin 10 (a 4.7K resistor is necessary)

const uint8_t ds18S20ID      = 0x10; // Maxim DS18S20 digital Thermometer device
const uint8_t ds2406ID       = 0x12; // Maxim DS2406+ digital switch
const uint8_t ds1822ID       = 0x22; // Maxim DS1822 digital Thermometer device
const uint8_t ds18b20ID      = 0x28; // Maxim DS18B20 digital Thermometer device
const uint8_t ds2762ID       = 0x30; // Maxim 2762 digital k-type thermocouple
const uint8_t max31850ID     = 0x3B; // MAX31850 K-type Thermocouple chip
const uint8_t max31855ID     = 0xAA; // Teensy 3.0 1-wire slave with MAX31855 K-type Thermocouple chip

void setup(void) 
{
  Serial.begin(11500);
  delay(5000);
}

uint8_t x = 0;
const uint8_t chips = 16;
uint8_t addr[chips][8] =
{
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0}
};

void loop(void)
{
  byte i;
  byte present = 0;
  byte data[12];
  float celsius, fahrenheit;
  
  if ( !ds.search(&addr[x][0]))
  {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    x = 0;
    return;
  }
  
  
  Serial.print(F("ROM "));
  Serial.print(x);
  Serial.print(F(" = "));
  for( i = 0; i < 8; i++) {
    Serial.print(F("0x"));
    if(addr[x][i] < 0x10) Serial.print(F("0"));
    Serial.print(addr[x][i], HEX);
    Serial.print(F(" "));
  }
  
  
  if (OneWire::crc8(addr[x], 7) != addr[x][7]) {
      Serial.print("  !!CRC is not valid!!");
      return;
  }

  // the first ROM byte indicates which chip
  switch (addr[x][0]) {
    case ds18S20ID:
      Serial.println("  DS18S20 Thermoneter");
      break;
    case ds2406ID:
      Serial.println("  DS2406+ Digital Switch");
      break;
    case ds1822ID:
      Serial.println("  DS1822 Thermoneter");
      break;
    case ds18b20ID:
      Serial.println("  DS18B20 Thermoneter");
      break;
    case ds2762ID:
      Serial.println("  DS2762 Thermocouple");
      break;
    case max31850ID:
      Serial.println("  MAX31850 Thermocouple");
      break;
    case max31855ID:
      Serial.println("  MAX31855 Thermocouple");
      break;
    default:
      Serial.println("  Unknown family device.");
      return;
  }
  x++;
  
  delay(100);
}
