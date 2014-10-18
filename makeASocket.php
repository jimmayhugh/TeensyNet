<?php
$getMaxChips        = "1";
$showChip           = $getMaxChips + 1;
$getChipCount       = $showChip + 1;
$getChipAddress     = $getChipCount + 1;
$getChipStatus      = $getChipAddress + 1;
$setSwitchState     = $getChipStatus + 1;
$getAllStatus       = $setSwitchState + 1;
$getChipType        = $getAllStatus + 1;
$getAllChips        = $getChipType + 1;
$getActionArray     = "A";
$updateActionArray  = "B";
$getActionStatus    = "C";
$getMaxActions      = "D";
$setActionSwitch    = "E";
$saveToEEPROM       = "F";
$getEEPROMstatus    = "G";
$getNewSensors      = "H";
$masterStop         = "I";
$getMaxPids         = "J";
$masterPidStop      = "K";
$getPidStatus       = "L";
$updatePidArray     = "M";
$getPidArray        = "N";
$setPidArray        = "O";
$useDebug           = "P";
$restoreStructures  = "Q";
$shortShowChip      = "R";
$updateChipName     = "S";
$showActionStatus   = "T";
$setAction          = "U";
$getMaxGLCDs        = "V";
$getGLCDcnt         = "W";
$getGLCDstatus      = "X";
$setGLCD            = "Y";
$getStructAddr      = "Z";

$updateglcd1wName   = "a";
$resetGLCD          = "b";
$setDebugPort       = "c";

$resetTeensy        = 'r';

$displayMessage     = "w";
$clearAndReset      = "x";
$clearEEPROM        = "y";
$versionID          = "z";

$on  = "1";
$off = "0";
$tooHot =  "H";
$tooCold = "C";
$socBufSize = 2048;

$dummyAddr="0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00";

error_reporting(E_ALL);

  /* Get the port for the WWW service. */
//  $service_port = getservbyname('www','tcp');

  /* Get the file address for the target host. */
//  $address = "/tmp/teensypi";

$service_port = '192.168.1.13';
$port_address = 8888;
   

function makeASocket()
{
error_reporting(~E_WARNING);
 
  if(!($socket = socket_create(AF_INET, SOCK_DGRAM, 0)))
  {
      $errorcode = socket_last_error();
      $errormsg = socket_strerror($errorcode);
       
      die("Couldn't create socket: [$errorcode] $errormsg \n");
  }
//  echo "Socket created \n"; 
  return $socket;
}
?>

