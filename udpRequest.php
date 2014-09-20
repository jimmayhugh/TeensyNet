<?php
$getMaxChips        = "1";
$showChip           = $getMaxChips + 1;
$getChipCount       = $showChip + 1;
$getChipAddress     = $getChipCount + 1;
$getChipStatus      = $getChipAddress + 1;
$setSwitchState     = $getChipStatus + 1;
$getAllStatus       = $setSwitchState + 1;
$getChipType        = $getAllStatus + 1;
$updateBonjour      = $getChipType + 1;
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

$displayMessage     = "w";
$clearAndReset      = "x";
$clearEEPROM        = "y";
$versionID          = "z";

$on  = "1";
$off = "0";
$tooHot =  "H";
$tooCold = "C";
$socBufSize = 2048;
$udpUpdate = 2000;

$dummyAddr="0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00";

error_reporting(E_ALL);

function udpRequest($service_port, $port_address, $in)
{

$myPID = getmypid();

error_reporting(~E_WARNING);

$fileName = "/var/log/teensynet/udp_".$service_port."_".$port_address.".log";   
$socBufSize = 2048;

  if(!($socket = socket_create(AF_INET, SOCK_DGRAM, 0)))
  {
      $errorcode = socket_last_error();
      $errormsg = socket_strerror($errorcode);
       
      die("Couldn't create socket: [$errorcode] $errormsg \n");
  }


  if(!socket_set_option($socket,SOL_SOCKET,SO_RCVTIMEO,array("sec"=>2,"usec"=>0)))
  {
      die("Couldn't set Socket Receive Timeout\n");
  }

  if( ! $fr = fopen($fileName, "a+") )
  {
	  die("Could not open file:".$fileName."\n");
  }else{
    fwrite($fr, "\n");
    fwrite($fr, date("D M j G:i:s Y"));
  } 

  $maxTries = 5;
  
  do{
    $result = "";
    fwrite($fr, "\nservice_port = $service_port, port_address = $port_address, pid = $myPID\n");
    socket_sendto($socket, $in , strlen($in) , 0 , $service_port , $port_address);
    fwrite($fr, "command sent: ".$in);

    if(socket_recvfrom($socket, $result, $socBufSize, MSG_WAITALL, $service_port, $port_address) === -1)
    {
    	fwrite($fr, socket_strerror(socket_last_error($socket)));
    }
      
    $resultStr = "";
      
    if($result === "=")
    {
      $in = "=";
      fwrite($fr, "large packet started: ".$in."\n");
      socket_sendto($socket, $in , strlen($in) , 0 , $service_port , $port_address);
      socket_recvfrom($socket, $result, $socBufSize, MSG_WAITALL, $service_port, $port_address);
      fwrite($fr, "first packet received: ".$result."\n");

      
      do{
        $resultStr .= $result;
        $result = "";
        fwrite($fr, "requesting next segment: ".$in."\n");
        socket_sendto($socket, $in , strlen($in), 0 , $service_port , $port_address);
        socket_recvfrom($socket, $result, $socBufSize, MSG_WAITALL, $service_port, $port_address);
        fwrite($fr, "next packet received: ".$result."\n");
      }while($result !== '+');
      
      if($result === '+')
      {
        fwrite($fr, "final segment received: ".$result."\n");
        $result = "";
      }
      
      $in = "";
    }else{
      $resultStr .= $result;
      $result = "";
    }
    $trimmedResultStr = trim($resultStr);
    $maxTries--;
  }while($trimmedResultStr === "ERROR" && $maxTries > 0);
  
  if($maxTries === 0)
  {
    fwrite($fr, "maxTries exceeded on PID# $myPID, I'm killing myself\n");
    fclose($fr);
    socket_close($socket);
    die();
  }

	if($resultStr === "")
	{
  	fwrite($fr, "NO Packet Received:\n");
	}else{
  	fwrite($fr, "packet received:\n".$resultStr);
  }
  fclose($fr);

  socket_close($socket);
  return $resultStr;
}
?>

