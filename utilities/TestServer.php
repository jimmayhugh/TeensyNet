<?php
 
//Reduce errors
error_reporting(~E_WARNING);

$resetDebug      = 0x00000001;              // 0x00000001; //       1
$pidDebug        = ($resetDebug      << 1); // 0x00000002; //       2
$eepromDebug     = ($pidDebug        << 1); // 0x00000004; //       4
$chipDebug       = ($eepromDebug     << 1); // 0x00000008; //       8
$findChipDebug   = ($chipDebug       << 1); // 0x00000010; //      16
$serialDebug     = ($findChipDebug   << 1); // 0x00000020; //      32
$udpDebug        = ($serialDebug     << 1); // 0x00000040; //      64
$wifiDebug       = ($udpDebug        << 1); // 0x00000080; //     128
$udpHexBuff      = ($wifiDebug       << 1); // 0x00000100; //     256
$chipNameDebug   = ($udpHexBuff      << 1); // 0x00000200; //     512
$actionDebug     = ($chipNameDebug   << 1); // 0x00000400; //     1024
$lcdDebug        = ($actionDebug     << 1); // 0x00000800; //     2048
$crcDebug        = ($lcdDebug        << 1); // 0x00001000; //     4096
$ds2762Debug     = ($crcDebug        << 1); // 0x00002000; //     8192
$bonjourDebug    = ($ds2762Debug     << 1); // 0x00004000; //    16384
$ethDebug        = ($bonjourDebug    << 1); // 0x00008000; //    32768
$udpTimerDebug   = ($ethDebug        << 1); // 0x00010000; //    65532
$glcdSerialDebug = ($udpTimerDebug   << 1); // 0x00020000; //   131064 GLCD debug on serial port
$glcdDebug       = ($glcdSerialDebug << 1); // 0x00040000; //   131064 GLCD debug on GLCD
$wdDebug         = ($glcdDebug       << 1); // 0x00080000; //   262128 Watchdog 
$chipStatusLED   = ($wdDebug         << 1); // 0x00100000; //   524256 ChipStatus LED
$glcd1WLED       = ($chipStatusLED   << 1); // 0x00200000; //  1048312 glcd1W LED

 
//Create a UDP socket
if(!($sock = socket_create(AF_INET, SOCK_DGRAM, 0)))
{
    $errorcode = socket_last_error();
    $errormsg = socket_strerror($errorcode);
     
    die("Couldn't create socket: [$errorcode] $errormsg \n");
}

if(!(socket_set_option($sock, SOL_SOCKET, SO_KEEPALIVE, 1)))
{
  echo "socket_set_option SO_KEEPALIVE failed\n";
}else{
  $option = socket_get_option($sock, SOL_SOCKET, SO_KEEPALIVE);
  echo "SO_KEEPALIVE = ".$option."\n";
}

if(($options = socket_get_option($sock, SOL_SOCKET, SO_RCVTIMEO, array('sec'=>5,'usec'=>0))) === FALSE)
{
  echo "socket_set_option for SO_RCVTIMEO failed\n";
}else{
  echo "SO_RCVTIMEO = 5 seconds\n";
}

 
$count = 0;
$remote_ip = '192.168.1.';
$remote_port = '2652';
$secCnt = 0;

//Do some communication, this loop can handle multiple clients

$secCnt = 0;
// get the last value for the IP

    echo 'Final IP Value (1 - 254): ';
    $in = fgets(STDIN);
    $remote_ip .= $in;
    echo "Using address $remote_ip";

while(1)
{    
    //Take some input to send
    echo 'Enter a message to send : ';
    $in = fgets(STDIN);
    $out = $in."\n";

    socket_sendto($sock, $in , 100 , 0 , $remote_ip , $remote_port);
    socket_recvfrom($sock, $buf, 2048, MSG_WAITALL, $remote_ip, $remote_port);
    
    $resultStr = "";
    if($buf === "=")
    {
      $in = "=";
      socket_sendto($sock, $in , 100 , 0 , $remote_ip , $remote_port);
      socket_recvfrom($sock, $buf, 2048, MSG_WAITALL, $remote_ip, $remote_port);

      while($buf !== '+')
      {
        $resultStr .= $buf;
        socket_sendto($sock, $in , 100 , 0 , $remote_ip , $remote_port);
        socket_recvfrom($sock, $buf, 2048, MSG_WAITALL, $remote_ip, $remote_port);
      }
    }else{
      $resultStr .= $buf;
    }
//    $resultStr1 = trim($resultStr);
//    $resultStr = str_replace("\n\n", "\n", $resultStr1);
//    echo "\$buf: $buf\n";
    echo "\$resultStr: \n$resultStr\n";
//    echo "\$resultStr1: $resultStr1\n";
}
 
socket_close($sock);
?>
