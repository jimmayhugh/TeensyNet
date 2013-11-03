<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
  <?php 
    include_once("udpRequest.php");
    include_once("accessDatabase.php");
    include_once("header.html");
    
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
  ?> 
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
    <title> Sensor /Switch Setup </title>
    <link rel="stylesheet" type="text/css" href="style.css"/>
    <script type="text/javascript" src="//ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js">
    </script>
    <?php
      $updateScript =
      "<script>
        $(document).ready(function() {
          $(\"#container\").load(\"updateStatus.php?netID=$netID&service_port=$service_port&port_address=$port_address&netName=$netName\");
          var refreshId = setInterval(function() {
            $(\"#container\").load(\"updateStatus.php?netID=$netID&service_port=$service_port&port_address=$port_address&netName=$netName\");
            }, 1000);
          $.ajaxSetup({ cache: false });
        });
    </script>";
    echo $updateScript;
    ?>
  </head>
  <body>
<!-- Table for Main Body -->
    <table width="100%" border="2" cellspacing="0" cellpadding="2">
      <tr>
        <td>
          <?php 
            include ("menu.php");
            if(isset($_POST["restoreall"]) && $_POST["restoreall"] === "restoreall")
            {
              $h2Str = "<h2>All Data Restored</h2>";
              $query = "SELECT * FROM chipNames WHERE `netID`='$netID'";
              $result=mysqli_query($link,$query);
              while($nameObj = mysqli_fetch_object($result))
              {
                $in = $updateChipName." ".$nameObj->address." ".$nameObj->name."\n";
//                echo $in."<br />";
		            $out = udpRequest($service_port, $port_address, $in);                
              }
              mysqli_free_result($result);
              
              $query = "SELECT * FROM action WHERE `netID`=$netID";
              // echo "query = ".$query."<br />";
              $result=mysqli_query($link,$query);
/*               
              if($result === FALSE)
              {
                $queryResultStr = "<br />query ".$query." failed<br />";
              }else{
                $queryResultStr = "<br />query ".$query." success ";
                $queryCntStr = mysqli_num_rows($result);
                $queryResultStr .= $queryCntStr." rows returned<br/>";
              }
*/
              while($actionObj = mysqli_fetch_object($result))
              {
                // echo $row[0].", ".$row[1].", ".$row[2].", ".$row[3]."<br />";
                // echo $row[4].", ".$row[5].", ".$row[6].", ".$row[7]."<br />";
                // echo $row[8].", ".$row[9].", ".$row[10].", ".$row[11]."<br />";
		            // $updateArrayData1 = $row[1]." 0 ".$row[2]." ";
		            // $updateArrayData2 = $row[4]." ".$row[5]." ".$row[3]." ";
		            // $updateArrayData3 = $row[7]." ".$row[8]." ".$row[6]." ";
		            $updateArrayData1 = $actionObj->active." 0 ".$actionObj->tempAddr." ";
		            $updateArrayData2 = $actionObj->tcTrigger." ".$actionObj->tcDelay." ".$actionObj->tcAddr." ";
		            $updateArrayData3 = $actionObj->thTrigger." ".$actionObj->thDelay." ".$actionObj->thAddr." ";

		            $in = $updateActionArray." ".$actionObj->id." 1 ".$updateArrayData1."\n";
		            $out = udpRequest($service_port, $port_address, $in);
		            $out1 = $out;
//		            echo "out1 = ".$out1."<br />";

		            $in = $updateActionArray." ".$actionObj->id." 2 ".$updateArrayData2."\n";
		            $out = udpRequest($service_port, $port_address, $in);
		            $out2 = $out;
//		            echo "out2 = ".$out2."<br />";

		            $in = $updateActionArray." ".$actionObj->id." 3 ".$updateArrayData3."\n";
		            $out = udpRequest($service_port, $port_address, $in);
		            $out3 = $out;
//		            echo "out3 = ".$out3."<br />";
		          }
		          mysqli_free_result($result);
		          
		          $query = "SELECT * from pid where `enabled`=1 AND netID=$netID";
              $result=mysqli_query($link,$query);
              while($obj = mysqli_fetch_object($result))
              {
                $updatePidStr = "M ".$obj->id." ".$obj->enabled." ".$obj->tempAddr." ".$obj->setpoint." ".$obj->switchAddr." ".$obj->kp." ".$obj->ki." ".$obj->kd." ".$obj->direction." ".$obj->windowSize."\n";
//                echo "<br />updatePidStr = ".$updatePidStr."<br />";
                $pidIn = $updatePidStr;
		            $pidOut = udpRequest($service_port, $port_address, $pidIn);
//                echo "pidIn = ".$pidIn.", length = ".strlen($pidIn)."<br />";
//                echo "pidOut = ".$pidOut."<br />";
              }
		          mysqli_free_result($result);
              
		          $in = $saveToEEPROM."\n";
		          $out = udpRequest($service_port, $port_address, $in);
//		          echo "saveToEEPROM = ".$out."<br/>";
		          sleep(2);
            }else if(isset($_POST["checknew"]) && $_POST["checknew"] === "checknew")
            {
              $in = $getNewSensors."\n";
              $eepromStatus = udpRequest($service_port, $port_address, $in);
            }else if(isset($_POST["dbclear"]) && $_POST["dbclear"] === "dbclear")
            {
              $dbClearStr = "DELETE FROM `action` WHERE `netID`='$netID'";
              $result = mysqli_query($link, $dbClearStr);
              $dbClearStr = "DELETE FROM `chipNames` WHERE `netID`='$netID'";
              $result = mysqli_query($link, $dbClearStr);
              $dbClearStr = "DELETE FROM `pid` WHERE 1";
              $result = mysqli_query($link, $dbClearStr);
              $dbClearStr = "DELETE FROM `actionGraph` WHERE `netID`='$netID'";
              $result = mysqli_query($link, $dbClearStr);
              $dbClearStr = "DELETE FROM `pidGraph` WHERE `netID`='$netID'";
              $result = mysqli_query($link, $dbClearStr);
              $in = $clearEEPROM."\n";
              $clearAndResetStatus = udpRequest($service_port, $port_address, $in);              
              $in = $clearAndReset."\n";
              echo "$clearAndResetStatus<br />";
              $clearAndResetStatus = udpRequest($service_port, $port_address, $in);
              echo "$clearAndResetStatus<br />";
            }
          ?>
        </td>
      </tr>
      <tr>
        <td  align="center" colspan="6">
          <table width="100%" border="1" cellspacing="0" cellpadding="2">
            <tr>
              <td align="center" colspan="6">
                <?php
                  echo "<h2><font color = \"blue\">$netName<br />Check For New Chips / Restore All Action Settings</font></h2>"
                ?>  
              </td>
            </tr>
            <tr>
              <td>
                <table width="100%" border="1" cellspacing="0" cellpadding="2">
                  <tr>
                    <td align="center" width="50%">
                      <font color="red">
                        USE THIS BUTTON WITH EXTREME CARE!!
                        <br />
                      </font>
                      <?php
                        $checkNewStr =
                        "
                        <form method=\"post\" action=\"CheckForNewRestore.php\">
                          <input type=\"hidden\" name=\"checknew\" value=\"checknew\">
                          <input type=\"hidden\" name=\"actionCnt\" value=\"".$x."\">
                          <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
                          <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
                          <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">
                          <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
                          <input type=\"submit\" value=\"Scan For New Chips\">
                        </form>
                      <font color=\"red\">
                        ";
                        echo $checkNewStr;
                      ?>
                        USING THIS BUTTON TURNS OFF ALL SWITCHES AND CLEARS ALL ACTIONS!!
                        <br />
                        USE THIS BUTTON WITH EXTREME CARE!!
                        <br />              
                      </font>      
                    </td>
                    <td align="center">
                      <?php
                        $checkDBActiveStr = "SELECT * from `chipNames` WHERE `address` != \"0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00\" AND `netID`='$netID'";
                        $result = mysqli_query($link, $checkDBActiveStr);
                        $nameCnt = mysqli_num_rows($result);
                        mysqli_free_result($result);
                        $checkDBActiveStr = "SELECT * from `action` WHERE `tempAddr` != \"0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00\" AND `netID`='$netID'";
                        $result = mysqli_query($link, $checkDBActiveStr);
                        $actionCnt = mysqli_num_rows($result);
                        mysqli_free_result($result);
                        $checkDBActiveStr = "SELECT * from `pid` WHERE `tempAddr` != \"0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00\" AND `netID`='$netID'";
                        $result = mysqli_query($link, $checkDBActiveStr);
                        $pidCnt = mysqli_num_rows($result);
                        mysqli_free_result($result);
                        $restoreAllStr = 
                          "
                            <form method=\"post\" action=\"CheckForNewRestore.php\">
                              <input type=\"hidden\" name=\"restoreall\" value=\"restoreall\">
                              <input type=\"hidden\" name=\"actionCnt\" value=\"".$x."\">
                              <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
                              <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
                              <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">
                              <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">";
                        if(($pidCnt > 0) || ($actionCnt > 0) || ($nameCnt > 0))
                        {
                          $restoreAllStr .= 
                            "<input type=\"submit\" value=\"RESTORE ALL\">
                             </form>
                            ";
                        }else{
                          $restoreAllStr .= 
                            "<input type=\"submit\" value=\"RESTORE ALL\" disabled>
                              </form>
                            ";
                        }
                        echo $restoreAllStr;
                      ?>
                    </td>
                  </tr>
                </table>
              </td>
            </tr>
          </table>      
        </td>
      </tr>  
      <tr>
        <td align="center" colspan="6">
          <font color="red">
            USE THIS BUTTON WITH EXTREME CARE!!
            <br />              
            USING THIS BUTTON COMPLETELY RESETS THE SYSTEM AND ALL SETTINGS!!
            <br />
          </font>
          <form method="post" action="CheckForNewRestore.php">
            <input type="hidden" name="actionCnt" value=.$x.>
            <input type="hidden" name=\"service_port\" value=\"".$service_port."\">
            <input type="hidden" name=\"port_address\" value=\"".$port_address."\">
            <input type="hidden" name=\"netName\" value=\"".$netName."\">
            <input type="hidden" name=\"netID\" value=\"".$netID."\">
            <input type="hidden" name="dbclear" value="dbclear">
            <input type="submit" value="CLEAR AND RESET SYSTEM">
          </form>
          <font color="red">
            USING THIS BUTTON COMPLETELY RESETS THE SYSTEM AND ALL SETTINGS!!
            <br />
            USE THIS BUTTON WITH EXTREME CARE!!
            <br />
          </font>              
        </td>
      </tr>
      <tr>
        <td align="center" >
            <?php
              echo $queryResultStr;
            ?>
        </td>
      </tr>
      <tr>
        <td>
          <div id="container">
          </div>
        </td>
      </tr>
      <tr>
        <td>
        <?php 
          include ("menu.php");
        ?>
        </td>
      </tr>
    </table>
  </body>
</html>
