<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<?php
  include_once("accessDatabase.php");
  include_once("udpRequest.php");
  
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

// Teensy3.0 Action Array indexes
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
  $escapedTempName="";
  $escapedTcName="";
  $escapedThName="";
  
  
  if( (isset($_POST["netID"]) && $_POST["netID"] >= 0) )
  {
    $netID = $_POST["netID"];
    $service_port = $_POST["service_port"];
    $port_address = $_POST["port_address"];
    $netName = $_POST["netName"];
    $actionCnt = $_POST["actionCnt"];
  }    
  if(isset($_GET["netID"]) && $_GET["netID"] >= 0) 
  {
    $netID = $_GET["netID"];
    $service_port = $_GET["service_port"];
    $port_address = $_GET["port_address"];
    $netName = $_GET["netName"];
  }
  
//    echo "SensorStatus.php: \$netID = ".$netID.", \$service_port = ".$service_port.", \$port_address = ".$port_address.", \$netName = ".$netName."<br />";

  if($netID >= 0)
  {
    $headStr = "
      <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">
      <title> Sensor / Switch Update </title>
      <link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"/>
      <script type=\"text/javascript\" src=\"//ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js\"></script>
      <!-- <script type=\"text/javascript\" src=\"js/jquery.js\"></script> -->
      <style>
      input[type='text'] { font-size: 18px; text-align: center;}
      input:focus, textarea:focus{background-color: lightgrey;}
      select[type='text'] { font-size: 18px; text-align: center;}
      </style>";
  }else{
    $headStr = "
      <meta charset=\"UTF-8\">
      <meta http-equiv=\"refresh\" content=\"1;url=http://localhost/Test/index.php\">
      <script type=\"text/javascript\">
          window.location.href = \"http://localhost/Test/index.php\"
      </script>";
  }

  if(isset($_POST["update"]) && $_POST["update"] === "update")
  {
    $tempAddress = $_POST["tempAddress"];
    $tcSwitchAddr = $_POST["tcSwitchAddr"];
    $thSwitchAddr = $_POST["thSwitchAddr"];
    $tcTemp = $_POST["tcTemp"];
    $tcDelay = $_POST["tcDelay"];
    $thTemp = $_POST["thTemp"];
    $thDelay = $_POST["thDelay"];
    $actionCnt = $_POST["actionCnt"];
    $actionEnable = $_POST["enable"];
    $lcd = $_POST["lcd"];
    
    $updateQuery = "SELECT * FROM action WHERE id='".$actionCnt."' AND netID='".$netID."'";
    $updateQueryResult = mysqli_query($link, $updateQuery);
    
    $tempAddrQuery = "SELECT name FROM chipNames WHERE address='".$tempAddress."' AND netID='".$netID."'";
    echo "\$tempAddrQuery = ".$tempAddrQuery."<br />";
    $tempAddrResult = mysqli_query($link, $tempAddrQuery);
    $tempAddrObj = mysqli_fetch_object($tempAddrResult);
    $updateArrayData1 = $actionEnable." 0 ".$tempAddress." ";
    echo "\$updateArrayData1 = ".$updateArrayData1."<br />";
    
    $tcAddrQuery = "SELECT name FROM chipNames WHERE address='".$tcSwitchAddr."' AND netID='".$netID."'";
    $tcAddrResult = mysqli_query($link, $tcAddrQuery);
    $tcAddrObj = mysqli_fetch_object($tcAddrResult);
    $updateArrayData2 = $tcTemp." ".$tcDelay." ".$tcSwitchAddr." ";
    echo "\$updateArrayData2 = ".$updateArrayData2."<br />";
    
    $thAddrQuery = "SELECT name FROM chipNames WHERE address='".$thSwitchAddr."' AND netID='".$netID."'";
    $thAddrResult = mysqli_query($link, $thAddrQuery);
    $thAddrObj = mysqli_fetch_object($thAddrResult);
    $updateArrayData3 = $thTemp." ".$thDelay." ".$thSwitchAddr." ";
    echo "\$updateArrayData3 = ".$updateArrayData3."<br />";
    
    $in = $updateActionArray." ".$actionCnt." 1 ".$updateArrayData1."\n";
    echo "\$in1 = ".$in."<br />";
    $out = udpRequest($service_port, $port_address, $in);
    $out1 = $out;

    $in = $updateActionArray." ".$actionCnt." 2 ".$updateArrayData2."\n";
    echo "\$in2 = ".$in."<br />";
    $out = udpRequest($service_port, $port_address, $in);
    $out2 = $out;

    $in = $updateActionArray." ".$actionCnt." 3 ".$updateArrayData3."\n";
    echo "\$in3 = ".$in."<br />";
    $out = udpRequest($service_port, $port_address, $in);
    $out3 = $out;

    $in = $saveToEEPROM."\n";
    $out = udpRequest($service_port, $port_address, $in);
    sleep(2);

    $escapedTempName = mysqli_real_escape_string ($link , $tempName);
    $escapedTcName = mysqli_real_escape_string ($link , $tcName);
    $escapedThName = mysqli_real_escape_string ($link , $thName);
//                $query = "REPLACE INTO  action SET active='".$actionEnable."',tempAddr='".$tempAddrObj->address."',tcAddr='".$tcAddrObj->address."',tcTrigger='".$_POST["tcTemp"]."',tcDelay='".$_POST["tcDelay"]."',thAddr='".$thAddrObj->address."',thTrigger='".$_POST["thTemp"]."',thDelay='".$_POST["thDelay"]."',lcd='".$_POST["lcd"]."' WHERE id='".$_POST["actionCnt"]."' AND netID='".$netID."'";
    if($updateQueryResult != NULL && mysqli_num_rows($updateQueryResult) > 0)
    {
      $query = "UPDATE  action SET id='".$actionCnt."',active='".$actionEnable."',tempAddr='".$tempAddress."',tcAddr='".$tcSwitchAddr."',tcTrigger='".$tcTemp."',tcDelay='".$tcDelay."',thAddr='".$thSwitchAddr."',thTrigger='".$thTemp."',thDelay='".$thDelay."',lcd='".$lcd."',netID='".$netID."' WHERE id='".$actionCnt."' AND netID='".$netID."'";
    }else{
      $query = "INSERT INTO  action SET id='".$actionCnt."',active='".$actionEnable."',tempAddr='".$tempAddress."',tcAddr='".$tcSwitchAddr."',tcTrigger='".$tcTemp."',tcDelay='".$tcDelay."',thAddr='".$thSwitchAddr."',thTrigger='".$thTemp."',thDelay='".$thDelay."',lcd='".$lcd."',netID='".$netID."'";
    } 

    echo $query."<br />";
    $result=mysqli_query($link,$query);
/*
    if($result === TRUE)
    {
      echo "query success";
    }else{
      echo "query failed";
    }
*/
    $h2Str = "<h2><font color = \"red\">".$netName."<br />Action Data Updated</font></h2>";
//    echo $h2Str;
    mysqli_free_result($tempAddrResult);
    mysqli_free_result($tcAddrResult);
    mysqli_free_result($thAddrResult);
    mysqli_free_result($updateQueryResult);
    
    $headStr = "
      <meta charset=\"UTF-8\">
      <meta http-equiv=\"refresh\" content=\"1;url=http://localhost/Test/ActionStatus.php?netID=$netID&service_port=$service_port&port_address=$port_address&netName=$netName\">
      <script type=\"text/javascript\">
          window.location.href = \"http://localhost/Test/ActionStatus.php?netID=$netID&service_port=$service_port&port_address=$port_address&netName=$netName\"
      </script>";

  }else{ // entry from Action Status.php
    $actionCnt = $_POST["actionCnt"];
//    echo "\$actionCnt = ".$actionCnt." <br />";
    $h2Str = "<h2><font color = \"blue\">Action Data<br />".$netName."</font></h2>";
//    echo $h2Str;

    $in = "$getChipCount\n";
    $out = udpRequest($service_port, $port_address, $in);
    $maxChipCnt = $out;
    for($scCnt=0; $scCnt < $maxChipCnt; $scCnt++)
    {
      $in = $showChip.$scCnt."\n";
      $out = udpRequest($service_port, $port_address, $in);
      $chipArray = explode(" ", $out);
      $chipAddressArray = explode(",", $chipArray[0]);
      switch($chipAddressArray[$chipTypeStr])
      {
        case "0x28":
        case "0x30":
        case "0xAA":
        {
          $tempOptionStr .= "<option value=\"$chipArray[$chipAddressStr]\">$chipArray[$chipNameStr]</option>";
          break;
        }
        
        case "0x12":
        {
          $switchOptionStr .= "<option value=\"".$chipArray[$chipAddressStr]."\">".$chipArray[$chipNameStr]."</option>";
          break;
        }
        
      }
    }
    
    $in = $getActionArray.$actionCnt."\n";
    $out = udpRequest($service_port, $port_address, $in);
    $actionArray = explode(" ", $out);

    $query = "select * from action where id=".$actionCnt." AND netID=".$netID;
    if(($result = mysqli_query($link,$query)) === FALSE)
    {
      echo "No Matching Action Array<br />";
    }
    $actionObj = mysqli_fetch_object($result);
    $containerStr .= "<form method=\"post\" action=\"ActionDataWithMySQL.php\">
            <table border=\"2\" cellspacing=\"0\" cellpadding=\"10\">
           <tr>
            <td align=\"center\" colspan=\"3\">
              Make your changes and press \"SUBMIT\"<br /><input type=\"submit\" value=\"SUBMIT\">
            </td>
          </tr>
          <tr>
            <td align=\"center\" colspan=\"3\">
              <input type=\"hidden\" name=\"update\" value=\"update\">
              <input type=\"hidden\" name=\"actionCnt\" value=\"".$actionCnt."\">
                Action Array #".$actionCnt."
                <br />";
    if($actionArray[$aaEnabled] === "1")
    {
      $containerStr .= "<font color=\"green\"><strong>ENABLED&nbsp;&nbsp;</strong></font>
                        <input type=\"radio\" name=\"enable\" value=\"1\" checked>Enable&nbsp;&nbsp;</input>
                        <input type=\"radio\" name=\"enable\" value=\"0\">Disable</input>";
    }else{
      $containerStr .= "<font color=\"red\"><strong>DISABLED&nbsp;&nbsp;</strong></font>
                        <input type=\"radio\" name=\"enable\" value=\"1\">Enable&nbsp;&nbsp;</input>
                        <input type=\"radio\" name=\"enable\" value=\"0\" checked>Disable</input>";
    }
    $containerStr .= "      </td></tr>";
    $tempAddrArray = explode(",", $actionArray[$aaTempAddr]);
    if( ($tempAddrArray[0] === "0x30") ||
        ($tempAddrArray[0] === "0x28") ||
        ($tempAddrArray[0] === "0xAA")
      )
    {
      $tempAddrQueryStr = "SELECT * FROM chipNames where address='".$actionArray[$aaTempAddr]."' AND netID='".$netID."'";
      $tempAddrQueryResult = mysqli_query($link, $tempAddrQueryStr);
      $tempAddrQueryObj = mysqli_fetch_object($tempAddrQueryResult);
      $containerStr .= "<tr>
              <td align=\"center\" colspan=\"3\">
                <table width=\"100%\" border=\"0\">
                  <tr>
                      <td align=\"center\">Temperature Sensor Name:
                          <select type=\"text\" name=\"tempAddress\">";
                            if($tempAddrQueryObj->name === "")
                            {
                              $containerStr .= "                 <option selected value=\"".$actionArray[$aaTempAddr]."\">".$tempAddrQueryObj->name."</option>";
                            }else{
                              $containerStr .= "                 <option selected value=\"".$tempAddrQueryObj->address."\">".$tempAddrQueryObj->name."</option>";
                            }
       $containerStr .= "<option value=\"UNASSIGNED\">UNASSIGNED</option>
                             ".$tempOptionStr."
                           </select>
                        </td>
                      </tr>
                  </table>
                </td>
              </tr>";
    }else{
      $containerStr .= "<tr>
              <td align=\"center\" colspan=\"3\">
              Temperature Sensor Name:
                  <select type=\"text\" name=\"tempAddress\">
                    <option value=\"UNASSIGNED\">UNASSIGNED</option>
                    ".$tempOptionStr."
                  </select>
              </td>
            </tr>";
    }
    $containerStr .= "<tr>
            <td align=\"center\" colspan=\"3\">
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
              <input type=\"text\" size=\"10\" name=\"tcTemp\" value=\"".$actionArray[$aaTCtemp]."\">
            </td>";
    $tcAddressArray = explode(",", $actionArray[$aaTCaddr]);
    if($tcAddressArray[0] === "0x12")
    {
      $tcAddrQueryStr = "SELECT * FROM chipNames where address='".$actionArray[$aaTCaddr]."' AND netID='".$netID."'";
      $tcAddrQueryResult = mysqli_query($link, $tcAddrQueryStr);
      $tcAddrQueryObj = mysqli_fetch_object($tcAddrQueryResult);
      $containerStr .= "<td align=\"center\">
              <select type=\"text\" name=\"tcSwitchAddr\">";
      if($tcAddrQueryObj->address === "")
      {
        $containerStr .= "<option selected value=\"".$actionArray[$aaTCaddr]."\">".$actionArray[$aaTCaddr]."</option>";
      }else{        
        $containerStr .= "<option selected value=\"".$tcAddrQueryObj->address."\">".$tcAddrQueryObj->name."</option>";
      }
      $containerStr .= "<option value=\"UNASSIGNED\">UNASSIGNED</option>
                ".$switchOptionStr."
              </select>
            </td>";
    }else{
      $containerStr .= "<td align=\"center\">
              <select type=\"text\" name=\"tcSwitchAddr\">
                <option value=\"UNASSIGNED\">UNASSIGNED</option>
                ".$switchOptionStr."
              </select>
            </td>";
    }          
    $containerStr .= "<td align=\"center\">
            <input type=\"text\" size=\"10\" name=\"tcDelay\" value=\"".$actionArray[$aaTCdelay]."\">
          </td>
        </tr>"; 
    $containerStr .= "<tr>
            <td align=\"center\" colspan=\"3\">
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
      $containerStr .= "<td align=\"center\">
              <select type=\"text\" name=\"thSwitchAddr\">
                <option selected value=\"".$thAddrQueryObj->address."\">".$thAddrQueryObj->name."</option>
                <option value=\"UNASSIGNED\">UNASSIGNED</option>
                ".$switchOptionStr."
              </select>
            </td>";
    }else{
      $containerStr .= "<td align=\"center\">
              <select type=\"text\" name=\"thSwitchAddr\">
                <option value=\"UNASSIGNED\">UNASSIGNED</option>
                ".$switchOptionStr."
              </select>
            </td>";
    }          
    $containerStr .= "<td align=\"center\">
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
  }
  
  echo $headStr;
?>
</head>
  <body>
    <?php 
      include ("header.html");
    ?> 
    <!-- Table for Main Body -->
    <table width="100%" border="1" cellspacing="0" cellpadding="1">
      <tr>
        <td valign="top" align="left" width="150">
        <?php 
        include ("menu.php");
        ?>
        </td>
      </tr>
      <tr>
        <td align="center" border="1">
          <div id="title">
            <?php
              echo $h2Str;
            ?>
          </div>
        </td>
      </tr>
      <tr>
        <td align="center" border="1">
          <div id="container">
            <?php
              echo $containerStr;
            ?>
          </div>
        </td>
      </tr>
      <tr>
        <td valign="top" align="left" width="150">
        <?php 
        include ("menu.php");
        ?>
        </td>
      </tr>
    </table>
    <?php
      mysqli_free_result($result);
      mysqli_close($link);
    ?>
  </body>
</html>
 
