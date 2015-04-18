<?php

  include_once("udpRequest.php");
  include_once("accessDatabase.php");
  if(isset($_POST["netID"]) && $_POST["netID"] >= 0)
  {
    $netID = $_POST["netID"];
    $service_port = $_POST["service_port"];
    $port_address = $_POST["port_address"];
    $netName = $_POST["netName"];
//      echo "CheckForNewRestore.php: \$netID = ".$netID.", \$service_port = ".$service_port.", \$port_address = ".$port_address.", \$netName = ".$netName."<br />";
  }

  if(isset($_GET["netID"]) && $_GET["netID"] >= 0)
  {
    $netID = $_GET["netID"];
    $service_port = $_GET["service_port"];
    $port_address = $_GET["port_address"];
    $netName = $_GET["netName"];
//      echo "CheckForNewRestore.php: \$netID = ".$netID.", \$service_port = ".$service_port.", \$port_address = ".$port_address.", \$netName = ".$netName."<br />";
  }
  $pidTempName = "";
  $pidSwitchName = "";

  
  $in = $getEEPROMstatus."\n";
	$eepromStatus = udpRequest($service_port, $port_address, $in);
 
  $in = $getMaxPids."\n";
  $chipX = udpRequest($service_port, $port_address, $in);
  
  $pidNum = trim($chipX);
  
  $in = $getPidStatus."\n";
  $chipX = udpRequest($service_port, $port_address, $in);
 
  $chipY = explode(";", $chipX);
  $chipYcount = count($chipY);

/* // Debug   
  echo "eepromStatus = ".$eepromStatus."<br />";
  echo "maxPIDs =".$pidNum."<br />";
  echo "chipYcount =".$chipYcount."<br />";
  
  for($x=0; $x < $chipYcount; $x++)
  {
    echo "chipY[".$x."] = ".$chipY[$x]."<br />";
  }
*/

  $bodyStr="<table width=\"100%\" border=\"2\" cellspacing=\"0\" cellpadding=\"5\">
              <tr>
                <td align=\"center\" colspan=\"5\">
                  <h2><font color = \"blue\">PID STATUS<br />$netName </font></h2>
                </td>
               <tr>
                <td align=\"center\" colspan=\"5\">
                  </form>
                  <font color=\"red\">
                    USE THIS BUTTON ONLY WHEN NECESSARY!!
                    <br />
                    </font>
                  <form method=\"post\" action=\"PidStatus.php\">
                    <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
                    <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
                    <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">
                    <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
                    <input type=\"hidden\" name=\"masterPidStop\" value=\"masterPidStop\">
                    <input type=\"submit\" id=\"masterStop\" value=\"Stop All Pids\">
                  </form>
                  <font color=\"red\">
                    USE THIS BUTTON ONLY WHEN NECESSARY!!
                    <br />
                    </font>
                      ";
    $bodyStr.="</td>
              </tr>
              <tr>";
  
//  for($x=0;$x<5;$x++)
  for($x=0; $x<$pidNum; $x++)
  {
    
    $chipXArray = explode(" ", $chipY[$x]);
    
    $pidEnabledStr      = trim($chipXArray[0]);
    $pidTempAddrStr     = trim($chipXArray[1]);
    $pidTempNameStr     = trim($chipXArray[2]);
    $pidTempStatusStr   = trim($chipXArray[3]);
    $pidSetPointStr     = trim($chipXArray[4]);
    $pidSwitchAddrStr   = trim($chipXArray[5]);
    $pidSwitchNameStr   = trim($chipXArray[6]);
    $pidSwitchStatusStr = trim($chipXArray[7]);
    $pidKpStr           = trim($chipXArray[8]);
    $pidKiStr           = trim($chipXArray[9]);
    $pidKdStr           = trim($chipXArray[10]);
    $pidDirectionStr    = trim($chipXArray[11]);
    $pidWindowSizeStr   = trim($chipXArray[12]);
/*    
    $bodyStr .= 
      "pid".$x." pidEnabledStr      = ".$pidEnabledStr."<br />
       pid".$x." pidTempAddrStr     = ".$pidTempAddrStr."<br />
       pid".$x." pidTempNameStr     = ".$pidTempNameStr."<br />
       pid".$x." pidTempStatusStr   = ".$pidTempStatusStr."<br />
       pid".$x." pidSetPointStr     = ".$pidSetPointStr."<br />
       pid".$x." pidSwitchAddrStr   = ".$pidSwitchAddrStr."<br />
       pid".$x." pidSwitchNameStr   = ".$pidSwitchNameStr."<br />
       pid".$x." pidSwitchStatusStr = ".$pidSwitchStatusStr."<br />
       pid".$x." pidKpStr           = ".$pidKpStr."<br />
       pid".$x." pidKiStr           = ".$pidKiStr."<br />
       pid".$x." pidKdStr           = ".$pidKdStr."<br />
       pid".$x." pidDirectionStr    = ".$pidDirectionStr."<br />
       pid".$x." pidWindowSizeStr   = ".$pidWindowSizeStr."<br />
       <br /><br/>";    

    if($pidSwitchNameStr === "_____UNASSIGNED_____"){$pidSwitchName = "PID".$x." Switch ";}
*/
    $bodyStr.= 
    "<div id=\"pid".$x."\">
      <td valign=\"top\" align=\"center\">
        <table border=\"1\" width=\"100%\" cellspacing=\"0\" cellpadding=\"10\">
          <tr>
            <td align=\"center\" colspan=\"4\">
              <form name=\"pidInfo\" method=\"post\" action=\"PidSetup.php\">
                <input type=\"hidden\" name=\"pidCnt\" value=\"".$x."\">
                <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
                <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
                <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
                <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
                <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">
                <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
                <input type=\"submit\" value=\"MODIFY\">
              </form>
           </td>
          <tr>
            <td align=\"center\" colspan=\"4\">";
    if($pidTempAddrStr === "0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00")
    {
        $bodyStr .= 
           "<font color=\"blue\"><strong>UNASSIGNED</strong></font>
                <form name=\"pidInfo\" method=\"post\" action=\"PidStatus.php\">
                  <input type=\"hidden\" name=\"pidCnt\" value=\"".$x."\">
                  <input type=\"hidden\" name=\"pidEnable\" value=\"pidEnable\">
                  <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
                  <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
                  <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">
                  <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
                  <input type=\"submit\" value=\"ENABLE\" disabled>
                </form>
               <form method=\"post\" action=\"plotPidData.php\">
                 <input type=\"hidden\" name=\"pidGraphId\" value=\"".$x."\">
                 <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
                 <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
                 <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">
                 <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
                 <input type=\"submit\" value=\"PIDGRAPH\" disabled>
               </form>";
/*        $bodyStr .= "<font color=\"red\"><strong>
                        <br \>Assign valid entries<br \>to enable.<br \>
                     </strong></font><font size=\"-1\"><br ></font>";*/
    }else{        
      if($pidEnabledStr === "0")
      {
        $bodyStr .= 
           "<font color=\"red\"><strong>DISABLED</strong></font>
                <form name=\"pidInfo\" method=\"post\" action=\"PidStatus.php\">
                  <input type=\"hidden\" name=\"pidCnt\" value=\"".$x."\">
                  <input type=\"hidden\" name=\"pidEnable\" value=\"pidEnable\">
                  <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
                  <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
                  <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">
                  <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
                  <input type=\"submit\" value=\"ENABLE\">
                </form>
                <form method=\"post\" action=\"plotPidData.php\">
                  <input type=\"hidden\" name=\"pidGraphId\" value=\"".$x."\">
                  <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
                  <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
                  <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">
                  <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
                  <input type=\"submit\" value=\"PIDGRAPH\" disabled>
                </form>";
       }else if($pidEnabledStr === "1"){
        $bodyStr .= 
          "<font color=\"green\"><strong>ENABLED</strong></font>
                <form name=\"pidInfo\" method=\"post\" action=\"PidStatus.php\">
                  <input type=\"hidden\" name=\"pidCnt\" value=\"".$x."\">
                  <input type=\"hidden\" name=\"pidDisable\" value=\"pidDisable\">
                  <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
                  <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
                  <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">
                  <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
                  <input type=\"submit\" value=\"DISABLE\">
                </form>
                <form method=\"post\" action=\"plotPidData.php\">
                  <input type=\"hidden\" name=\"pidGraphId\" value=\"".$x."\">
                  <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
                  <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
                  <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">
                  <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
                  <input type=\"submit\" value=\"PIDGRAPH\">
                </form>";
      }else{
        $bodyStr .=
          "<font color=\"yellow\"><strong>UNKNOWN = ".$pidEnabledStr."<br /></strong></font>";
      }
    }

    $bodyStr .= 
       "</td>
     </tr>
<!--
     <tr>
       <td align=\"center\" colspan=\"4\">
         <div style=\"vertical-align:middle; min-height:50px;\">
           <font size=\"5\" color=\"blue\"><strong>"
            .$pidTempNameStr."
           </strong></font>
       </td>
     </tr>
-->     
     <tr>
      <td align=\"center\" colspan=\"4\">";
    if($pidTempStatusStr >= -76 && $pidTempStatusStr < 215)
    {
      if($pidTempStatusStr > $pidSetPointStr)
      {
        $fontColor = "red";
      }else if($pidTempStatusStr < $pidSetPointStr){
        $fontColor = "blue";
      }else if($pidTempStatusStr === $pidSetPointStr){
        $fontColor = "green";
      }
      
      $bodyStr .=
        "<div style=\"vertical-align:middle; min-height:85px; max-height:80px;\">
           <font size=\"5\" color=\"".$fontColor."\"><strong>
           ".$pidTempNameStr."
           </strong></font><br />
          <font size=\"10\" color=\"".$fontColor."\"><strong>".$pidTempStatusStr."&deg;</strong></font>
         </div>";
    }else{
      $bodyStr .=
        "<div style=\"vertical-align:middle; min-height:85px;\">
          <font color=\"red\" size=\"5\"><strong>UNASSIGNED</strong></font>
         </div>";
    }
    $bodyStr .= 
       "</td>
     </tr>
     <tr>
      <td align=\"center\" colspan=\"4\">
        <div style=\"vertical-align:middle; min-height:50px;\">
          <font size=\"5\" color=\"blue\"><strong>
            Set Point<br />".$pidSetPointStr."
          </strong></font>
        </div>
      </td>
     </tr>
     <tr>
      <td align=\"center\" colspan=\"4\">";
    if($pidSwitchStatusStr === "N")
    {
        $bodyStr .=
           "<div style=\"vertical-align:middle; min-height:85px; max-height:85px;\">
              <font size=\"5\" color=\"blue\"><strong>
                ".$pidSwitchNameStr."
              </strong></font><br />              
              <font size=\"10\" color=\"green\"><strong>ON</strong></font>
            </div>";
    }else if($pidSwitchStatusStr === "F"){
        $bodyStr .= 
           "<div style=\"vertical-align:middle; min-height:85px; max-height:85px;\">
              <font size=\"5\" color=\"blue\"><strong>
                ".$pidSwitchNameStr."
              </strong></font><br />              
              <font size=\"10\" color=\"red\"><strong>OFF</strong></font>
            </div>";
    }else{
        $bodyStr .= "
            <div style=\"vertical-align:middle; min-height:85px; max-height:90px;\">
              <font size=\"5\" color=\"red\"><strong>UNASSIGNED</strong></font>
            </div>";
    }
    $bodyStr .= "
      </td>
    </tr>
    <tr>
      <td align=\"center\" colspan=\"4\">
        <div style=\"vertical-align:middle; min-height:50px;\">
          <font size=\"5\" color=\"blue\"><strong>PID Variables<br />
          Kp:&nbsp;".$pidKpStr."<br />Ki:&nbsp;".$pidKiStr."<br />Kd:&nbsp;".$pidKdStr."
          </strong></font>
        </div>
    ";
    $bodyStr .="
      </td>
    </tr>
    <tr>
      <td align=\"center\" colspan=\"4\">
        <div style=\"vertical-align:middle; min-height:50px;\">
          <font size=\"5\" color=\"blue\"><strong>Direction<br /></strong></font>";
          if($pidDirectionStr === "0")
          {
            $bodyStr .="<font size=\"5\" color=\"red\"><strong>Forward</strong></font>";
          }else{
            $bodyStr .="<font size=\"5\" color=\"blue\"><strong>Reverse</strong></font>";
          }
    $bodyStr .="</div>";
    $bodyStr .= "
      </td>
    </tr>
    <tr>
      <td align=\"center\" colspan=\"4\">
        <div style=\"vertical-align:middle; min-height:50px;\">
          <font size=\"5\" color=\"blue\"><strong>PID Window Size<br />".$pidWindowSizeStr."</strong></font>
        </div>";
    $bodyStr .= "</td></tr>\n";
    $bodyStr .= "</table></td></div>\n";
    if($x === 3 || $x === 7) {  $bodyStr .= "</tr><tr>\n";}
  }
  $bodyStr .= "</tr>\n</table>";

  echo $bodyStr;
  mysqli_close($link);

?>
