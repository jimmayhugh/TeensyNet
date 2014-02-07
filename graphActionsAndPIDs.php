<?php
  include_once("/var/www/htdocs/accessDatabase.php");
  include_once("/var/www/htdocs/udpRequest.php");

  $arrayActive = 0;
  $arrayTemp   = 1;
  $arrayTC     = 2;
  $arrayTH     = 3;
  $arrayTcTemp = 4;
  $arrayThTemp = 5;

  $fileName = "/var/log/teensynet/graph_update.log";   

  if( ! $fr = fopen($fileName, "a+") )
  {
	  die("Could not open file:".$fileName."\n");
  }else{
    fwrite($fr, "\n");
    fwrite($fr, date("D M j G:i:s Y"));
    fwrite($fr, "\n");
  } 
  
  $actionTime = date("U");
  $deviceQuery = "SELECT * FROM `netDevices` WHERE `netActive`=1";
  fwrite($fr, $deviceQuery."\n");
  echo $deviceQuery."\n";
  $devResult = mysqli_query($link, $deviceQuery);
  if($devResult === FALSE)
  {
    fwrite($fr, "device query Failed\n");
    die("device query Failed\n");
  }
  $devCnt = mysqli_num_rows($devResult);
  fwrite($fr, $devCnt." device rows retrieved\n");
  echo $devCnt." device rows retrieved\n";
  while($devObj = mysqli_fetch_object($devResult))
  {
    $actionQuery = "SELECT * FROM `action` WHERE `netID`=".$devObj->netID." AND `active`=1";
    fwrite($fr, $actionQuery."\n");
    echo $actionQuery."\n";
    $actionResult = mysqli_query($link, $actionQuery);
    if($actionResult === FALSE)
    {
      fwrite($fr, "action query Failed\n");
//      die("action query Failed\n");
    }
    while($actionObj = mysqli_fetch_object($actionResult))
    {
    // Get and store Action data to graph database table
      $in = $showActionStatus.$actionObj->id."\n";
      $out = udpRequest($devObj->service_port, $devObj->port_address, $in);

      $tOut = trim($out);
      if($tOut === "Invalid Command")
      {
        continue;
      }

      echo "\$in = $in, \$out = $out \n";

      $chipArray = explode(",", $out);
      if($chipArray[$arrayActive] === '0')
      {
        continue;
      }
      
      $actionTemp = $chipArray[$arrayTemp];
      
      if($chipArray[$arrayTC] === 'F')
      {
        $actionTC = "OFF";
      }else if($chipArray[$arrayTC] === 'N'){
        $actionTC = "ON";
      }else{
        $actionTC = "NONE";
      }
      
      if($chipArray[$arrayTH] === 'F')
      {
        $actionTH = "OFF";
      }else if($chipArray[$arrayTH] === 'N'){
        $actionTH = "ON";
      }else{
        $actionTH = "NONE";
      }
      
      $actionTooCold = trim($chipArray[$arrayTcTemp]);
      $actionTooHot = trim($chipArray[$arrayThTemp]);
//    echo "Action #".$x." = ".$x."\n";
      $actionInsertQuery = "INSERT INTO actionGraph SET `id`=$actionObj->id,`time`=$actionTime,`temp`=$actionTemp,`tcTemp`=$actionTooCold,`tcSwitch`='$actionTC',";
      $actionInsertQuery .= "`thTemp`=$actionTooHot,`thSwitch`='$actionTH',`netID`=$devObj->netID,`netName`='$devObj->netName'";
      fwrite($fr, $actionInsertQuery."\n");
      echo $actionInsertQuery."\n";
      $actionInsertResult = mysqli_query($link, $actionInsertQuery);
      if($actionInsertResult === FALSE)
      {
        fwrite($fr, "Action Insert Failed\n");
//        die("Action Insert Failed\n");
      }else{
        fwrite($fr, "Action Insert Success\n");
      }
      mysqli_free_result($actionInsertResult);
    }
    mysqli_free_result($actionResult);
  }
  mysqli_free_result($devResult);
/*
// Get and Store PID data to graph database table
  
  $newSocket = udpRequest($service_port, $address);
  $in = "$getPidStatus\n";
  socket_write($newSocket, $in, strlen($in));
  $out = socket_read($newSocket, $socBufSize);
  socket_close($newSocket);

  $pidTime = date("U");
  $pidOutArray = explode(";", $out);

  for($x=0; $x < count($pidOutArray); $x++)
  {
    $pidArray = explode(",", $pidOutArray[$x]);
    if($pidArray[0] === '0')
    {
      continue;
    }
    $pidID = $x;
    $pidTemp = $pidArray[1];
    $pidSetPoint = $pidArray[2];
    if($pidArray[3] === 'F')
    {
      $pidSwitch = "0";
    }else if($pidArray[3] === 'N'){
      $pidSwitch = "1";
    }else{
      $pidSwitch = "-1";
    }
    $pidDirection = $pidArray[7];
//    echo "PID #".$x." = ".$pidArray[$x]."\n";
    $query = "INSERT INTO pidGraph VALUES('".$pidID."','".$pidSetPoint."','".$pidTemp."','".$pidSwitch."','".$pidDirection."','".$pidTime."')";
//    echo $query."\n";
    mysqli_query($link, $query);
  }
*/
  mysqli_close($link);
  fclose($fr);
?>
