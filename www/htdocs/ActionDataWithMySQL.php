<?php
/*
ActionDataWithMySQL.php
Version 0.0.48
Last Modified 12/01/2014
By Jim Mayhugh


Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

This software uses multiple libraries that are subject to additional
licenses as defined by the author of that software. It is the user's
and developer's responsibility to determine and adhere to any additional
requirements that may arise.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******Note To Self
**  The funky strings in this file are set up that way to make the resulting HTML file readable.
**  If you change things, make sure that you look at the HTML source that's generated to make sure it's still
**  reasonably readable.
******* End of Note To Self
*/
  $debugFile = fopen("/home/jimm/debug/actionDataWithMySql.txt", "w+");// creates a new file each time, "a+" appends
  $now = date("Y/m/d - h:i:sa");
  $debugStr = "$now<br />";
  include_once("accessDatabase.php");
  include_once("udpRequest.php");
  include_once("headStr.php");
  include_once("logoStr.php");
  include_once("menuStr.php");
  include_once("footStr.php");
  include_once("glcdDefines.php");
  
  $chipTypeStr    = 0;
  $chipAddressStr = 0;
  $chipNameStr    = 2;
  $tempOptionStr  = "";

  $actionID           =  0;
  $actionEnabled      =  1;
  $actionTempAddress  =  2;
  $actionTCaddress    =  3;
  $actionTCtrigger    =  4;
  $actionTCdelay      =  5;
  $actionTHaddress    =  6;
  $actionTHtrigger    =  7;
  $actionTHdelay      =  8;
  $actionLCD          =  9;
  $actionNetID        = 10;

// Teensy3.x Action Array indexes
  $aaEnabled  =  0;
  $aaTempAddr =  1;
  $aaTempName =  2;
  $aaTCtemp   =  3;
  $aaTCaddr   =  4;
  $aaTCname   =  5;
  $aaTCstatus =  6;
  $aaTCdelay  =  7;
  $aaTCmillis =  8;
  $aaTHtemp   =  9;
  $aaTHaddr   = 10;
  $aaTHname   = 11;
  $aaTCstatus = 12;
  $aaTHdelay  = 13;
  $aaTHmillis = 14;
  $aaLCD      = 15;
  
  $switchOptionStr = "";
  $escapedTempName = "";
  $escapedTcName = "";
  $escapedThName = "";

  $glcdActionStr = "";
  $containerStr = "";
  
  $lcdOptionStr="<option value=\"32\">LCD0</option>
                        <option value=\"33\">LCD1</option>
                        <option value=\"34\">LCD2</option>
                        <option value=\"35\">LCD3</option>
                        <option value=\"36\">LCD4</option>
                        <option value=\"37\">LCD5</option>
                        <option value=\"38\">LCD6</option>";
  
  if( (isset($_POST["netID"]) && $_POST["netID"] >= 0) )
  {
    $netID = $_POST["netID"];
    $service_port = $_POST["service_port"];
    $port_address = $_POST["port_address"];
    $netName = $_POST["netName"];
    $actionCnt = $_POST["actionCnt"];
    $actionAddr = trim($_POST["actionAddr"]);
    $debugStr .= "<br />\$_POST[] variables:
\$netID = $netID
\$service_port = $service_port
\$port_address = $port_address
\$netName = $netName
\$actionCnt = $actionCnt
\$actionAddr = $actionAddr
<br /><br />
";
  }
      
  if(isset($_GET["netID"]) && $_GET["netID"] >= 0) 
  {
    $netID = $_GET["netID"];
    $service_port = $_GET["service_port"];
    $port_address = $_GET["port_address"];
    $netName = $_GET["netName"];
    $debugStr .= "<br />\$_GET[] variables:
\$netID = $netID
\$service_port = $service_port
\$port_address = $port_address
\$netName = $netName
<br /><br />
";
  }
  
    $debugStr .= "SensorStatus.php: \$netID = $netID, \$service_port = $service_port, \$port_address = $port_address, \$netName = $netName<br />";

  if(isset($_POST["update"]) && $_POST["update"] === "update")
  {
  
    $debugStr .= "Entry from updateActionStatusWithMySQL.php<br />";

    $tempAddress  = $_POST["tempAddress"];
    $tcSwitchAddr = $_POST["tcSwitchAddr"];
    $thSwitchAddr = $_POST["thSwitchAddr"];
    $tcTemp       = $_POST["tcTemp"];
    $tcDelay      = $_POST["tcDelay"];
    $thTemp       = $_POST["thTemp"];
    $thDelay      = $_POST["thDelay"];
    $actionCnt    = $_POST["actionCnt"];
    $actionEnable = $_POST["enable"];
    $lcd          = $_POST["lcd"];
    $plcd1w       = $_POST["plcd1w"];
    $lcd1w        = $_POST["lcd1w"];
    $pglcd        = $_POST["pglcd"];
    $glcd         = $_POST["glcd"];

    
    $debugStr .= "<br />Starting Update:
\$tempAddress = $tempAddress
\$tcSwitchAddr = $tcSwitchAddr
\$thSwitchAddr = $thSwitchAddr
\$tcTemp = $tcTemp
\$tcDelay = $tcDelay
\$thTemp = $thTemp
\$thDelay = $thDelay
\$actionCnt = $actionCnt
\$actionAddr = $actionAddr
\$actionEnable = $actionEnable
\$lcd = $lcd
\$lcd1w = $lcd1w
\$glcd = $glcd
\$pglcd = $pglcd<br /><br />
";

// update TeensyNet items set by updateArrayData
    $updateQuery = "SELECT * FROM action WHERE id='".$actionCnt."' AND netID='".$netID."'";
    $updateQueryResult = mysqli_query($link, $updateQuery);

    
    $tempAddrQuery = "SELECT name FROM chipNames WHERE address='".$tempAddress."' AND netID='".$netID."'";
    $debugStr .= "\$tempAddrQuery = $tempAddrQuery<br />";
    $tempAddrResult = mysqli_query($link, $tempAddrQuery);
    $tempAddrObj = mysqli_fetch_object($tempAddrResult);
    $updateArrayData1 = $actionEnable." 0 ".$lcd." ".$tempAddress." ";
    $debugStr .= "\$updateArrayData1 = ".$updateArrayData1."<br />";

    
    $tcAddrQuery = "SELECT name FROM chipNames WHERE address='".$tcSwitchAddr."' AND netID='".$netID."'";
    $tcAddrResult = mysqli_query($link, $tcAddrQuery);
    $tcAddrObj = mysqli_fetch_object($tcAddrResult);
    $updateArrayData2 = $tcTemp." ".$tcDelay." 0 ".$tcSwitchAddr." ";
    $debugStr .= "\$updateArrayData2 = ".$updateArrayData2."<br />";

    
    $thAddrQuery = "SELECT name FROM chipNames WHERE address='".$thSwitchAddr."' AND netID='".$netID."'";
    $thAddrResult = mysqli_query($link, $thAddrQuery);
    $thAddrObj = mysqli_fetch_object($thAddrResult);
    $updateArrayData3 = $thTemp." ".$thDelay." 0 ".$thSwitchAddr." ";
    $debugStr .= "\$updateArrayData3 = ".$updateArrayData3."<br />";

    
    $in = $updateActionArray." ".$actionCnt." 1 ".$updateArrayData1;
    $debugStr .= "\$updateActionArray$actionCnt in1 = ".$in."<br />";
    $out = trim(udpRequest($service_port, $port_address, $in));
    $debugStr .= "\$updateActionArray$actionCnt out1 = $out<br />";


    $in = $updateActionArray." ".$actionCnt." 2 ".$updateArrayData2;
    $debugStr .= "\$updateActionArray$actionCnt in2 = ".$in."<br />";
    $out = trim(udpRequest($service_port, $port_address, $in));
    $debugStr .= "\$updateActionArray$actionCnt out2 = $out <br />";


    $in = $updateActionArray." ".$actionCnt." 3 ".$updateArrayData3;
    $debugStr .= "\$updateActionArray$actionCnt in3 = ".$in."<br />";
    $out = trim(udpRequest($service_port, $port_address, $in));
    $debugStr .= "\$updateActionArray$actionCnt out3 = $out<br />";

// turn TeensyNet Action switches off if $actionEnable === 0
    if($actionEnable === "0")
    {
      $in = $setActionSwitch.$actionCnt.$tooHot."F";
      $killSwitch = trim(udpRequest($service_port, $port_address, $in));
      $in = $setActionSwitch.$actionCnt.$tooCold."F";
      $killSwitch = trim(udpRequest($service_port, $port_address, $in));
    }
    
// update 1-wire GLCD on TeensyNet
    $in = $getStructAddr."A".$actionCnt;
    $debugStr .= "\$getStructAddr in = $in<br />";
    $ActionAddress =  trim(udpRequest($service_port, $port_address, $in));
    $debugStr .= "\$ActionAddress out = $ActionAddress<br />";

    if(isset($_POST["glcd"]))
    {
      if($glcd != "NULL") //new value
      {
        $glcdArray = unserialize(urldecode($glcd));        
        $debugStr .= "\$glcdArray[0] = $glcdArray[0], \$glcdArray[1] = $glcdArray[1], \$glcdArray[2] = $glcdArray[2]<br />";
        $debugStr .= "Setting new glcd<br />";
        $in = $setGLCD." ".$glcdArray[0]." 3 ".$glcdArray[1]." ".$ActionAddress;
        $newGLCDarray[0] = $glcdArray[0];
        $newGLCDarray[1] = $glcdArray[1];
        $newGLCDarray[2] = $ActionAddress;
        $out = trim(udpRequest($service_port, $port_address, $in));
        $debugStr .= "\$setGLCD out - $out<br />";
        $newGLCD = urlencode(serialize($newGLCDarray));  
      }else{ // glcd is NULL
        if( (isset($_POST["pglcd"])) && ($pglcd != "NULL") && ($pglcd != "") )
        {
          $glcdArray = unserialize(urldecode($glcd));        
          $debugStr .= "\$glcdArray[0] = $glcdArray[0], \$glcdArray[1] = $glcdArray[1], \$glcdArray[2] = $glcdArray[2]<br />";          
          $in = $setGLCD." ".$glcdArray[0]." 3 ".$glcdArray[1]." 00000000";
          $out = trim(udpRequest($service_port, $port_address, $in));
          $debugStr .= "\$setGLCD out - $out<br />";
        }
        $debugStr .= "Setting new glcd to NULL<br />";
        $newGLCD = "NULL";
      }
      $debugStr .= "\$newGLCD = $newGLCD<br />";
      $debugStr .= "\$setGLCD in - $in<br />";
    }

    if(isset($_POST["pglcd"]))
    {
      $debugStr .= "\$pglcd = $pglcd<br />";
      if(($pglcd != "NULL") && ($pglcd != "") && ($pglcd != $newGLCD))
      {
        $pglcdArray = unserialize(urldecode($pglcd));        
        $debugStr .= "\$pglcdArray[0] = $pglcdArray[0], \$pglcdArray[1] = $pglcdArray[1], \$pglcdArray[2] = $pglcdArray[2]<br />";
        $debugStr .= "Setting new pglcd to NULL<br />";
        $in = $setGLCD." ".$pglcdArray[0]." 3 ".$pglcdArray[1]." 00000000";
        $debugStr .= "\$setGLCD in - $in<br />";
        $out = trim(udpRequest($service_port, $port_address, $in));
        $debugStr .= "\$setGLCD out - $out<br />";
      }
    }
    $pglcd = $newGLCD;
    $debugStr .= "Setting pglcd to newGLCD<br />";
    $debugStr .= "\$glcd = $glcd<br />";
    $debugStr .= "\$pglcd = $pglcd<br />";

// update 1-wire LCD on Teensynet

    if(isset($_POST["lcd1w"]))
    {
      $debugStr .= "Updating 1-Wire LCD, \$lcd1w = $lcd1w<br />";
      if($lcd1w != "NONE")
      {
        $in = $setLCD." ".$lcd1w." 01 ".$ActionAddress;
        $debugStr .= "\$setLCD in = $in<br />";
        $out =  trim(udpRequest($service_port, $port_address, $in));
        $debugStr .= "\$setLCD out = $out<br />";
      }else{
        $in = $getLCDcnt;
        $resetCnt =  trim(udpRequest($service_port, $port_address, $in));
        for($lcd1w = 0; $lcd1w < $resetCnt; $lcd1w++)
        {
          $in = $getLCDstatus.$lcd1w;
          $out = trim(udpRequest($service_port, $port_address, $in));
          $newLCDarray = explode(";", $out);
          if($newLCDarray[4] === $ActionAddress)
          {
            $in = $setLCD." ".$lcd1w." 00 00000000";
            $debugStr .= "Resetting 1-Wire LCD $lcd1w<br />";
            $debugStr .= "\$setLCD in = $in<br />";
            $out =  trim(udpRequest($service_port, $port_address, $in));
            $debugStr .= "\$setLCD out = $out<br />";
          }
        }
      }
    }
    
// save to i2cEEPROM
    $in = $saveToEEPROM;
    $out = trim(udpRequest($service_port, $port_address, $in));
    sleep(2);
    $debugStr .= "Saved to i2cEEPROM<br />";

// update MySQL
    $in = "$getGLCDstatus$glcdArray[0]";
    $debugStr .= "\$getGLCDstatus$glcdArray[0] in = $in<br />";
    $out = trim(udpRequest($service_port, $port_address, $in));
    $debugStr .= "\$getGLCDstatus$glcdArray[0] out = $out<br />";
    $glcdSarray = explode(";", $out);

    $escapedTempName = mysqli_real_escape_string ($link , $tempName);
    $escapedTcName = mysqli_real_escape_string ($link , $tcName);
    $escapedThName = mysqli_real_escape_string ($link , $thName);

    if($updateQueryResult != NULL && mysqli_num_rows($updateQueryResult) > 0)
    {
      $query = "UPDATE  action SET id='".$actionCnt."',active='".$actionEnable."',tempAddr='".$tempAddress."',tcAddr='".$tcSwitchAddr."',tcTrigger='".$tcTemp."',tcDelay='".$tcDelay."',thAddr='".$thSwitchAddr."',thTrigger='".$thTemp."',thDelay='".$thDelay."',lcd='".$lcd."',lcd1w='".$lcd1w."',glcd='".$glcdSarray[1]."-".$glcdArray[1]."',netID='".$netID."' WHERE id='".$actionCnt."' AND netID='".$netID."'";
    }else{
      $query = "INSERT INTO  action SET id='".$actionCnt."',active='".$actionEnable."',tempAddr='".$tempAddress."',tcAddr='".$tcSwitchAddr."',tcTrigger='".$tcTemp."',tcDelay='".$tcDelay."',thAddr='".$thSwitchAddr."',thTrigger='".$thTemp."',thDelay='".$thDelay."',lcd='".$lcd."',lcd1w='".$lcd1w."',glcd='".$glcdSarray[1]."-".$glcdArray[1]."',netID='".$netID."'";
    } 

    $debugStr .= $query."<br />";
    $result=mysqli_query($link,$query);

    if($result === TRUE)
    {
      $debugStr .= "update Action query success<br />";
    }else{
      $debugStr .= "update Action query failed<br />";
    }

    $updatedStr = "<br /><font color = \"red\">Action #".$actionCnt." Data Updated</font></h2>";
    mysqli_free_result($tempAddrResult);
    mysqli_free_result($tcAddrResult);
    mysqli_free_result($thAddrResult);
    mysqli_free_result($updateQueryResult);
  }
// end of updateActionStatusWithMySQL.php actions, update the webpage below

// entry from ActionStatus.php
    if(!(isset($_POST["update"])))
    {
      $debugStr .= "Entry from ActionStatus.php<br />";
    }else{
      $debugStr .= "Continued from updateActionStatusWithMySQL.php<br />";
    }
    $actionCnt = $_POST["actionCnt"];
    $debugStr .= "\$actionCnt = $actionCnt<br />";

    $h2Str = "<h2><font color = \"blue\">Action #".$actionCnt." Data<br />$netName</font>";

    if(isset($_POST["update"]) && $_POST["update"] === "update")
    {
      $h2Str .= $updatedStr;
    }else{
      $h2Str .= "</h2>";
    }


// setup Temp options
    $in = $getChipCount;
    $out = trim(udpRequest($service_port, $port_address, $in));
    $maxChipCnt = $out;
    for($scCnt=0; $scCnt < $maxChipCnt; $scCnt++)
    {
      $in = $showChip.$scCnt;
      $out = trim(udpRequest($service_port, $port_address, $in));
      $debugStr .= "\$showChip".$scCnt." = ".$out."<br />";
      $chipArray = explode(" ", $out);
      $chipAddressArray = explode(",", $chipArray[0]);
      switch($chipAddressArray[$chipTypeStr])
      {
        case "0x28":
        case "0x3B":
        case "0x30":
        case "0xAA":
        {
          $tempOptionStr .= "
                            <option value=\"$chipArray[$chipAddressStr]\">$chipArray[$chipNameStr]</option>";
          break;
        }
        
        case "0x12":
        {
          $switchOptionStr .= "
                    <option value=\"".$chipArray[$chipAddressStr]."\">".$chipArray[$chipNameStr]."</option>";
          break;
        }
        
      }
    }

// setup GLCD options
    $glcdActionStr .= "
<!-- setup GLCD options -->"
;
    $in = "$getGLCDcnt";
    $out = trim(udpRequest($service_port, $port_address, $in));
    $glcdCnt = $out;
    $debugStr .= "\$getGLCDcnt = $glcdCnt<br />";
    $in = $getStructAddr."A".$actionCnt;
    $debugStr .= "\$getStructAddr in = $in<br />";
    $ActionAddress =  trim(udpRequest($service_port, $port_address, $in));
    $debugStr .= "\$ActionAddress out = $ActionAddress<br />";
    $glcdActionStr .= "
                  </td>
                 </tr>
                 <tr>
                   <td align=\"center\" colspan=\"3\">
                     Assign 1-Wire GLCD: 
                     <select type=\"text\" name=\"glcd\"><br />";
    for($glcd = 0; $glcd < $glcdCnt; $glcd++)
    {
      $in = $getGLCDstatus.$glcd;
      $debugStr .= "\$getGLCDstatus".$glcd." in = ".$in."<br />";
      $out = trim(udpRequest($service_port, $port_address, $in));
      $debugStr .= "\$getGLCDstatus".$glcd." out = ".$out."<br />";
      $glcdArray = explode(";", $out);

      for($glcdActionCnt = 3; $glcdActionCnt < 7; $glcdActionCnt++)
      {
        $debugStr .= "Checking for match on $glcdActionCnt<br />";
        if(trim($glcdArray[$glcdActionCnt]) === trim($actionAddr))
        {
          $glcdActionCnt -= 3;
          $debugStr .= "MATCH on $glcdActionCnt<br />";
          $glcdVals[0] = $glcd;
          $glcdVals[1] = $glcdActionCnt;
          $glcdVals[2] = trim($actionAddr);
          $glcdActionStr .= "
                       <option selected value=\"";
          $glcdActionStr .= urlencode(serialize($glcdVals));
          $debugStr .= "\$glcd = $glcd<br />";
          if($glcdArray[1] === "NULL")
          {
            $glcdActionStr .= "\">GLCD".$glcd."-".$glcdActionCnt."</option><br />";
          }else{
            $glcdActionStr .= "\">".$glcdArray[1]."-".$glcdActionCnt."</option><br />";
          }
          break;
        }else{
          $debugStr .= "NO MATCH on $glcdActionCnt<br />";
        }
      }
      
      $glcdActionStr .= "
                       <option value=\"NULL\">NONE</option>";
      
      for($glcdActionCnt = 0, $glcdActionOffset = 3;
          $glcdActionCnt < 4;
          $glcdActionCnt++)
      {
        
        $glcdVals[0] = $glcd;
        $glcdVals[1] = $glcdActionCnt;
        $glcdVals[2] = $glcdArray[$glcdActionOffset];
        $glcdActionStr .= "
                       <option value=\"";
        $glcdStr = urlencode(serialize($glcdVals));
        $glcdActionStr .= $glcdStr;
        $debugStr .= "\$glcd = $glcd<br />";
        if($glcdArray[1] === "NULL")
        {
          $glcdActionStr .= "\">GLCD".$glcd."-".$glcdActionCnt."</option><br />";
        }else{
          $glcdActionStr .= "\">".$glcdArray[1]."-".$glcdActionCnt."</option><br />";
        }
        $glcdActionOffset++;
      }
    }
    $glcdActionStr .= "
                     </select>";
    if((!(isset($_POST["pglcd"]))) || ($pglcd === "") || ($pglcd === "NULL"))
    {
      if((!(isset($_POST["update"]))))
      {
        $glcdActionStr .="
                     <input type=\"hidden\" name=\"pglcd\" value=\"$glcdStr\">";
      }else{
        $glcdActionStr .="
                     <input type=\"hidden\" name=\"pglcd\" value=\"NULL\">";
      }
    }else{
      $glcdActionStr .="
                      <input type=\"hidden\" name=\"pglcd\" value=\"$pglcd\">";
    }
      $glcdActionStr .="      
                      <input type=\"hidden\" name=\"actionAddr\" value=\"$actionAddr\">     
                    </td>
                  </tr>";
          
// setup 1-wire LCD options
    $in = "$getLCDcnt";
    $out = trim(udpRequest($service_port, $port_address, $in));
    $lcd1wCnt = $out;
    $debugStr .= "\$lcd1wCnt = $lcd1wCnt<br />";
    for($lcd1w = 0; $lcd1w < $lcd1wCnt; $lcd1w++)
    {
      $in = "$getLCDstatus$lcd1w";
      $debugStr .= "\$getLCDstatus$lcd1w in = $in<br />";
      $out = trim(udpRequest($service_port, $port_address, $in));
      $debugStr .= "\$getLCDstatus$lcd1w out = $out<br />";
      $lcd1wArray = explode(";", $out);
      
      if((trim($lcd1wArray[3]) === "01") && (trim($lcd1wArray[4]) != NULL))
      {
        $lcd1wActionStr .= "
                              <option value=\"$lcd1w\">";
        if($lcd1wArray[2] === "__UNASSIGNED___")
        {
          $lcd1wActionStr .= "LCD1W-$lcd1w</option>";
        }else{
          $lcd1wActionStr .= "$lcd1wArray[2]</option>";
        }
        break;
      }
    }

    $lcd1wActionStr .= "
                              <option value=\"NONE\">NONE</option>";

    for($lcd1w = 0; $lcd1w < $lcd1wCnt; $lcd1w++)
    {
      $in = "$getLCDstatus$lcd1w";
      $debugStr .= "\$getLCDstatus$lcd1w in = $in<br />";
      $out = trim(udpRequest($service_port, $port_address, $in));
      $debugStr .= "\$getLCDstatus$lcd1w out = $out<br />";
      $lcd1wArray = explode(";", $out);
      
      $lcd1wActionStr .= "
                              <option value=\"$lcd1w\">";
      if($lcd1wArray[2] === "__UNASSIGNED___")
      {
        $lcd1wActionStr .= "LCD1W-$lcd1w</option>";
      }else{
        $lcd1wActionStr .= "$lcd1wArray[2]</option>";
      }
    }

    $in = $getActionArray.$actionCnt;
    $out = trim(udpRequest($service_port, $port_address, $in));
    $actionArray = explode(";", $out);


    $query = "select * from action where id=".$actionCnt." AND netID=".$netID;
    if(($result = mysqli_query($link,$query)) === FALSE)
    {
      $debugStr .= "No Matching Action Array<br />";
    }
    $actionObj = mysqli_fetch_object($result);
    $containerStr .= "
            <form method=\"post\" action=\"ActionDataWithMySQL.php\">
              <table border=\"2\" cellspacing=\"0\" cellpadding=\"10\">
                <tr>
                  <td align=\"center\" colspan=\"3\">
                    Make your changes and press \"SUBMIT\"<br />
                    <input type=\"submit\" value=\"SUBMIT\">
                  </td>
                </tr>
                <tr>
                  <td align=\"center\" colspan=\"3\">
                    <input type=\"hidden\" name=\"service_port\" value=\"$service_port\">
                    <input type=\"hidden\" name=\"port_address\" value=\"$port_address\">
                    <input type=\"hidden\" name=\"netName\" value=\"$netName\">
                    <input type=\"hidden\" name=\"netID\" value=\"$netID\">
                    <input type=\"hidden\" name=\"update\" value=\"update\">
                    <input type=\"hidden\" name=\"actionCnt\" value=\"$actionCnt\">
                  Action Array #$actionCnt<br />";
    if($actionArray[$aaEnabled] === "1")
    {
      $containerStr .= "
                    <font color=\"green\"><strong>ENABLED&nbsp;&nbsp;</strong></font>
                    <input type=\"radio\" name=\"enable\" value=\"1\" checked>Enable&nbsp;&nbsp;</input>
                    <input type=\"radio\" name=\"enable\" value=\"0\">Disable</input>";
    }else{
      $containerStr .= "
                    <font color=\"red\"><strong>DISABLED&nbsp;&nbsp;</strong></font>
                    <input type=\"radio\" name=\"enable\" value=\"1\">Enable&nbsp;&nbsp;</input>
                    <input type=\"radio\" name=\"enable\" value=\"0\" checked>Disable</input>";
    }
    $containerStr .= "
                  </td>
                </tr>
                <tr>
                  <td align=\"center\" colspan=\"3\">
                    Assign I2C LCD:
                      <select type=\"text\" name=\"lcd\">";
              if( ($actionArray[$aaLCD] >= "32") && ($actionArray[$aaLCD] <= "38") )
              {
                $lcdVal = $actionArray[$aaLCD] - 32;
                $containerStr .= "
                        <option value=\"".$actionArray[$aaLCD]."\">LCD".$lcdVal."</option>";
              }
              $containerStr .= "
                        <option value=\"0\">NONE</option>
                        $lcdOptionStr
                      </select>";
/*
    if($actionArray[$aaLCD] === "1")
    {
      $containerStr .= "<font color=\"green\"><strong>ENABLED&nbsp;&nbsp;</strong></font>
                        <input type=\"radio\" name=\"lcd\" value=\"1\" checked>Enable&nbsp;&nbsp;</input>
                        <input type=\"radio\" name=\"lcd\" value=\"0\">Disable</input>";
    }else{
      $containerStr .= "<font color=\"red\"><strong>DISABLED&nbsp;&nbsp;</strong></font>
                        <input type=\"radio\" name=\"lcd\" value=\"1\">Enable&nbsp;&nbsp;</input>
                        <input type=\"radio\" name=\"lcd\" value=\"0\" checked>Disable</input>";
    }
*/

    $containerStr .= $glcdActionStr;
    $containerStr .= "
                        <tr>
                          <td align=\"center\" colspan=\"3\">
<!-- setup 1-wire LCD options -->
                            Assign 1-Wire LCD:
                            <select type=\"text\" name=\"lcd1w\">
                              $lcd1wActionStr
                            </select>
                          </td>
                        </tr>";
        
    $tempAddrArray = explode(",", $actionArray[$aaTempAddr]);
    if( ($tempAddrArray[0] === "0x30") ||
        ($tempAddrArray[0] === "0x3B") ||
        ($tempAddrArray[0] === "0x28") ||
        ($tempAddrArray[0] === "0xAA")
      )
    {
      $tempAddrQueryStr = "SELECT * FROM chipNames where address='".$actionArray[$aaTempAddr]."' AND netID='".$netID."'";
      $debugStr .= "\$tempAddrQueryStr = $tempAddrQueryStr<br />";
      $tempAddrQueryResult = mysqli_query($link, $tempAddrQueryStr);
      $tempAddrQueryObj = mysqli_fetch_object($tempAddrQueryResult);
      $debugStr .= "\$tempAddrQueryObj->name = $tempAddrQueryObj->name<br />";
      $containerStr .= "<tr>
          <td align=\"center\" colspan=\"3\">
            <table width=\"100%\" border=\"0\">
              <tr>
                <td align=\"center\">Temperature Sensor Name:
<!-- setup Temp options -->
                  <select type=\"text\" name=\"tempAddress\">";
                            if($tempAddrQueryObj->name === "")
                            {
                              $containerStr .= "
                    <option selected value=\"".$actionArray[$aaTempAddr]."\">".$tempAddrQueryObj->name."</option>";
                            }else{
                              $containerStr .= "
                    <option selected value=\"".$tempAddrQueryObj->address."\">".$tempAddrQueryObj->name."</option>";
                            }
       $containerStr .= "
                    <option value=\"UNASSIGNED\">UNASSIGNED</option>$tempOptionStr
                  </select>
                </td>
              </tr>
            </table>
          </td>
        </tr>\n";
    }else{
      $containerStr .= "
        <tr>
          <td align=\"center\" colspan=\"3\">
            Temperature Sensor Name:
            <select type=\"text\" name=\"tempAddress\">
              <option value=\"UNASSIGNED\">UNASSIGNED</option>
              $tempOptionStr
            </select>
          </td>
        </tr>";
    }
    $containerStr .= "
        <tr>
          <td align=\"center\" colspan=\"3\">
<!-- setup Too Cold Switch options -->
            Too Cold Parameters
          </td>
        </tr>
      <tr>
        <td align=\"center\">
          Trigger Temp
        </td>
        <td align=\"center\">
          Name:
        </td>
        <td align=\"center\">
          Delay (Seconds)
        </td>
      </tr>
      <tr>
        <td align=\"center\">
          <input type=\"text\" size=\"10\" name=\"tcTemp\" value=\"$actionArray[$aaTCtemp]\">
        </td>";
    $tcAddressArray = explode(",", $actionArray[$aaTCaddr]);
    if($tcAddressArray[0] === "0x12")
    {
      $tcAddrQueryStr = "SELECT * FROM chipNames where address='".$actionArray[$aaTCaddr]."' AND netID='".$netID."'";
      $tcAddrQueryResult = mysqli_query($link, $tcAddrQueryStr);
      $tcAddrQueryObj = mysqli_fetch_object($tcAddrQueryResult);
      $containerStr .= "
                <td align=\"center\">
                  <select type=\"text\" name=\"tcSwitchAddr\">";
      if($tcAddrQueryObj->address === "")
      {
        $containerStr .= "
                    <option selected value=\"".$actionArray[$aaTCaddr]."\">".$actionArray[$aaTCaddr]."</option>";
      }else{        
        $containerStr .= "
                    <option selected value=\"".$tcAddrQueryObj->address."\">".$tcAddrQueryObj->name."</option>";
      }
      $containerStr .= "
                    <option value=\"UNASSIGNED\">UNASSIGNED</option>$switchOptionStr
                  </select>
                </td>";
    }else{
      $containerStr .= "
                <td align=\"center\">
                  <select type=\"text\" name=\"tcSwitchAddr\">
                    <option value=\"UNASSIGNED\">UNASSIGNED</option>$switchOptionStr
                  </select>
                </td>";
    }          
    $containerStr .= "
                <td align=\"center\">
                  <input type=\"text\" size=\"10\" name=\"tcDelay\" value=\"".$actionArray[$aaTCdelay]."\">
                </td>
              </tr>
              <tr>
                <td align=\"center\" colspan=\"3\">
<!-- setup Too Hot Switch options -->
                  Too Hot Parameters
                </td>
              </tr>
              <tr>
                <td align=\"center\">
                  Trigger Temp
                </td>
                <td align=\"center\">
                  Name:
                </td>
                <td align=\"center\">
                  Delay (Seconds)
                </td>
              </tr>
              <tr>
                <td align=\"center\">
                  <input type=\"text\" size=\"10\" name=\"thTemp\" value=\"".$actionArray[$aaTHtemp]."\">
                </td>";
    $thAddressArray = explode(",", $actionArray[$aaTHaddr]);
    if($thAddressArray[0] === "0x12")
    {
      $thAddrQueryStr = "SELECT * FROM chipNames WHERE address='".$actionArray[$aaTHaddr]."' AND  netID='".$netID."'";
      $thAddrQueryResult = mysqli_query($link, $thAddrQueryStr);
      $thAddrQueryObj = mysqli_fetch_object($thAddrQueryResult);
      $containerStr .= "
                <td align=\"center\">
                  <select type=\"text\" name=\"thSwitchAddr\">
                    <option selected value=\"".$thAddrQueryObj->address."\">".$thAddrQueryObj->name."</option>
                    <option value=\"UNASSIGNED\">UNASSIGNED</option>$switchOptionStr
                  </select>
                </td>";
    }else{
      $containerStr .= "
                <td align=\"center\">
                  <select type=\"text\" name=\"thSwitchAddr\">
                    <option value=\"UNASSIGNED\">UNASSIGNED</option>$switchOptionStr
                  </select>
                </td>";
    }          
    $containerStr .= "
                <td align=\"center\">
                  <input type=\"text\" size=\"10\" name=\"thDelay\" value=\"".$actionArray[$aaTHdelay]."\"
                </td>
              </tr> 
              <tr>
                <td align=\"center\" colspan=\"3\">
                  Make your changes and press \"SUBMIT\"<br />
                  <input type=\"hidden\" name=\"maxChipCnt\" value=\"".$maxChipCnt."\">
                  <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
                  <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
                  <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
                  <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">
                  <input type=\"submit\" value=\"SUBMIT\">
                </td>
              </tr>
            </table>
          </form>";

  mysqli_free_result($result);
  mysqli_close($link);

  $bodyStr ="
    <!-- Table for Main Body -->
    <table width=\"100%\" border=\"1\" cellspacing=\"0\" cellpadding=\"1\">
      <tr>
        <td>
          $menuStr
        </td>
      </tr>
      <tr>
        <td align=\"center\" border=\"1\">
          <div id=\"title\">
              $h2Str
          </div>
        </td>
      </tr>
      <tr>
        <td align=\"center\" border=\"1\">
          <div id=\"container\">$containerStr
          </div>
        </td>
      </tr>
      <tr>
        <td valign=\"top\" align=\"left\" width=\"150\">
        </td>
      </tr>
      <tr>
        <td>
          $menuStr
        </td>
      </tr>
    </table>";

  
  echo $headStr;
  echo $logoStr;
  echo $bodyStr;
//  echo $debugStr;
  echo $footStr;
  $deBugStr = str_replace("<br />","\n",$debugStr);
  $deBugStr .= "\n\n\n";
  fwrite($debugFile, $deBugStr);
  fclose($debugFile);
?>
 
