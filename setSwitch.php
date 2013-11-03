<?php

  include_once("udpRequest.php");

  if(isset($_GET["service_port"]) && isset($_GET["port_address"]) && isset($_GET["netName"])) 
    {
      $service_port=$_GET["service_port"];
      $port_address=$_GET["port_address"];
      $netName=$_GET["netName"];
//      echo "service_port = $service_port, port_address = $port_address, netName = $netName <br />";
    }

  $in = $setSwitchState.$_GET["switch"]."\n";
  $chipX = udpRequest($service_port, $port_address, $in);
  
  $result=trim($chipX);
  if($result === "N")
  {
    echo TRUE;
  }else if($result === "F"){
    echo FALSE;
  }

?>
