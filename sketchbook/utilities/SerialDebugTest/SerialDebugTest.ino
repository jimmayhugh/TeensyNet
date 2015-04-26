/* Serial Debug Test
** Version: 0.0.1
** Author:  Jim Mayhugh
** Date:    04/26/2015
**
** The pupose of this short test is to verify the two Serial channels
** used for debug purposes on the TeensyNet are operational.
**
** The IDE Serial Monitor is used on TeensyNet boards prior to Version 14
** The hardware ability to use either the IDE Serial Monitor or 
** alternate Serial2 was included in TeensyNet hardware version >= 14.
**
** When using Serial2, you'll need a USB to TTL serial adapter similar to this:
** http://smile.amazon.com/gp/product/B00F2F5HVK/ref=oh_aui_detailpage_o02_s00?ie=UTF8&psc=1
**
** With the above cable the connections are as follows:
** Black - GND
** White - RX2
** Green - TX2
** Red   - VCC
**
** Your connections may vary with a different cable.
**
** To test, compile and load this program onto the TeensyNet, open the IDE Serial Monitor, and
** use a terminal program such as Putty or Cutecom to connect to the appropriate device.
** In a Linux derivative, it should be something like /dev/ttyUSBx or /dev/ttyACMx, where x is a number.
** Make sure that the baud rate to both the IDE Serial Monitor and FTDI Monitor are set to 115200
*/

void setup()
{
  // reassign pins 26 and 31 to use the ALT3 configuration
  // which makes them  Serial port 2 Rx(26) and Tx(31)
  CORE_PIN26_CONFIG = PORT_PCR_MUX(3);
  CORE_PIN31_CONFIG = PORT_PCR_MUX(3);

  Serial.begin(115200);
  Serial.println("Serial2 begin");
  
  Serial2.begin(115200);
  Serial2.println("Serial2 begin");
}

void loop()
{
  for(uint32_t x; x < 0xffffffff; x++)
  {
    Serial.print(F("Serial: Hello World!! - "));
    Serial.println(x);
    delay(500);
    Serial2.print(F("Serial2: Hello World!! - "));
    Serial2.println(x);
    delay(500);
  }
}
