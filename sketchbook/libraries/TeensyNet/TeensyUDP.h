/*  TeensyUDP.h
    Version 0.01 0831/2014
    by Jim Mayhugh
*/

#ifndef TUDP_H
#define TUDP_H

// define UDP commands

const uint8_t getMaxChips        = '1';
const uint8_t showChip           = getMaxChips + 1;    // "2"
const uint8_t getChipCount       = showChip + 1;       // "3"
const uint8_t getChipAddress     = getChipCount + 1;   // "4"
const uint8_t getChipStatus      = getChipAddress + 1; // "5"
const uint8_t setSwitchState     = getChipStatus + 1;  // "6"
const uint8_t getAllStatus       = setSwitchState + 1; // "7"
const uint8_t getChipType        = getAllStatus + 1;   // "8"
const uint8_t updateBonjour      = getChipType + 1;    // "9"

const uint8_t getActionArray     = 'A'; // start of new serial command list
const uint8_t updateActionArray  = getActionArray + 1;    // "B"
const uint8_t getActionStatus    = updateActionArray + 1; // "C"
const uint8_t getMaxActions      = getActionStatus + 1;   // "D"
const uint8_t setActionSwitch    = getMaxActions + 1;     // "E"
const uint8_t saveToEEPROM       = setActionSwitch + 1;   // "F"
const uint8_t getEEPROMstatus    = saveToEEPROM + 1;      // "G"
const uint8_t getNewSensors      = getEEPROMstatus + 1;   // "H"
const uint8_t masterStop         = getNewSensors + 1;     // "I"
const uint8_t getMaxPids         = masterStop + 1;        // "J"
const uint8_t masterPidStop      = getMaxPids + 1;        // "K"
const uint8_t getPidStatus       = masterPidStop + 1;     // "L"
const uint8_t updatePidArray     = getPidStatus + 1;      // "M"
const uint8_t getPidArray        = updatePidArray + 1;    // "N"
const uint8_t setPidArray        = getPidArray + 1;       // "O"
const uint8_t useDebug           = setPidArray + 1;       // "P"
const uint8_t restoreStructures  = useDebug + 1;          // "Q"
const uint8_t shortShowChip      = restoreStructures + 1; // "R"
const uint8_t updateChipName     = shortShowChip + 1;     // "S"
const uint8_t showActionStatus   = updateChipName + 1;    // "T"
const uint8_t setAction          = showActionStatus + 1;  // "U"
const uint8_t getMaxGLCDs        = setAction + 1;         // "V"
const uint8_t getGLCDcnt         = getMaxGLCDs + 1;       // "W"
const uint8_t getGLCDstatus      = getGLCDcnt + 1;        // "X"
const uint8_t setGLCD            = getGLCDstatus + 1;     // "Y"
const uint8_t getStructAddr      = setGLCD + 1;           // "Z"

const uint8_t updateGLCD1wName   = 'a';
const uint8_t resetGLCD          = updateGLCD1wName + 1;  // "b"
const uint8_t setDebugPort       = resetGLCD + 1;         // "c"
const uint8_t getMaxLCDs         = setDebugPort + 1;      // "d"
const uint8_t getLCDcnt          = getMaxLCDs + 1;        // "e"
const uint8_t getLCDstatus       = getLCDcnt + 1;         // "f"
const uint8_t setLCD             = getLCDstatus + 1;      // "g"
const uint8_t updateLCD1wName    = setLCD + 1;            // "h"
const uint8_t resetLCD           = updateLCD1wName + 1;   // "i"

const uint8_t getDebug           = 'p';

const uint8_t resetTeensy        = 'r';

const uint8_t setTempType        = 't'; // 0 = Celsius 1 = Fahrenheit

const uint8_t displayMessage     = 'w';
const uint8_t clearAndReset      = 'x';
const uint8_t clearEEPROM        = 'y';
const uint8_t versionID          = 'z';


// end of serial commands

const uint8_t softSerialError  = 'X';
const uint8_t setSwitchON      = 'N';
const uint8_t setSwitchOFF     = 'F';
const uint8_t switchStatusON   = 'N';
const uint8_t switchStatusOFF  = 'F';
const uint8_t tooHotSwitch     = 'H';
const uint8_t tooColdSwitch    = 'C';
const uint8_t noChipPresent    = 0xFF;


// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;
#endif
