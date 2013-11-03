<?php

  include_once("udpRequest.php");

  $in = $setActionSwitch.$_GET["data"]."\n";
  $chipX = udpRequest($service_port, $port_address, $in);

  $result=trim($chipX);
  if($result === "N")
  {
    echo TRUE;
  }else if($result === "F"){
    echo FALSE;
  }

?>
