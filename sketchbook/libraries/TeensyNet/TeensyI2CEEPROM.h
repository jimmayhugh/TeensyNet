/*  TeensyI2CEEPROM.h
    Version 0.01 0831/2014
    by Jim Mayhugh
*/
#ifndef TI2CEE_H
#define TI2CEE_H

//I2CEEPROM Stuff
bool              i2cEepromReady                = FALSE;
uint16_t          i2cEeResult16;
uint8_t           i2cEeResult;
uint8_t           i2cIPResult[4]                = {0,0,0,0};

const uint8_t     I2CEEPROMidVal                = 0x55;   // Shows that an EEPROM update has occurred
const uint8_t     I2C0x50                       = 0x50;   // device address at 0x50
const uint8_t     I2C0x51                       = 0x51;   // device address at 0x51
const uint8_t     pageSize                      = 128;    // MicroChip 24LC512 buffer page
const uint32_t    I2CEEPROMsize                 = 0xFFFF; // MicroChip 24LC512
const uint16_t    I2CEEPROMidAddr               = 0x0005; // verify a previous I2CEEPROM write
const uint16_t    I2CEEPROMchipCntAddr          = 0x0100; // chips found during findchips()
const uint16_t    I2CEEPROMnumGLCDsAddr         = 0x0200; // 1-Wire glcds found during findchips()
const uint16_t    I2CEEPROMnum1wLCDsAddr        = 0x0300; // 1-Wire lcds found during findchips()
const uint16_t    I2CEEPROMbjAddr               = 0x0400; // start of Bonjour name buffer
const uint16_t    I2CEEPROMipAddr               = 0x0500; // start of IP address storage
const uint16_t    I2CEEPROMchipAddr             = 0x1000; // start address of chip structures
const uint16_t    I2CEEPROMactionAddr           = 0x3000; // start address of action structures
const uint16_t    I2CEEPROMpidAddr              = 0x5000; // start address of chip structures
const uint16_t    I2CEEPROMglcdAddr             = 0x7000; // start address of GLCD structures
const uint16_t    I2CEEPROMlcdAddr              = 0x9000; // start address of LCD structures

#endif
