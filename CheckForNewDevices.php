<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
    <title> Sensor /Switch Setup </title>
    <link rel="stylesheet" type="text/css" href="style.css"/>
    <script type="text/javascript" src="//ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js">
    </script>
<!--    <script>
      $(document).ready(function() {
        $("#container").load("updateStatus.php");
        var refreshId = setInterval(function() {
          $("#container").load('updateStatus.php');
          }, 1000);
        $.ajaxSetup({ cache: false });
      });
    </script>
-->
  </head>
  <body>
  <?php 
    include_once("udpRequest.php");
    include_once("accessDatabase.php");
    include_once("header.html");
//    echo "netID = ".$netID.", netName = ".$netName.", service_port = ".$service_port.", port_address = ".$port_address.", netActive = ".$netActive."<br />";
    
  ?> 
<!-- Table for Main Body -->
    <table width="100%" border="2" cellspacing="0" cellpadding="2">
      <tr>
        <td>
          <?php 
            include ("menu.php");
            $netID      = 0;
            $netName    = $netID+1;
            $service_port = $netName+1;
            $port_address    = $service_port+1;
            $netActive  = $port_address+1;
            if(isset($_POST["checknew"]) && $_POST["checknew"] === "checknew")
            {
              $dvNetworkDevicesStr = "SELECT * FROM `netDevices` WHERE netActive=1";
              $dvResult = mysqli_query($link, $dvNetworkDevicesStr);
              $dvCnt = mysqli_num_rows($dvResult);
              echo $dvCnt." TeensyNets found <br />";
              while($dvRow = mysqli_fetch_array($dvResult))
              {
                $dvStr = "<br />netID = ".$dvRow[$netID].", netName = ".$dvRow[$netName].", service_port = ".$dvRow[$service_port].", port_address = ".$dvRow[$port_address].", netActive = ".$dvRow[$netActive]."<br >";
                $dvIn = $clearEEPROM."\n";
                $dvStatus = udpRequest($dvRow[$service_port], $dvRow[$port_address], $dvIn);
                $dvStr .= $dvRow[$service_port]." Status: ".$dvStatus."<br />";
                sleep(1);
                $dvIn = $getNewSensors."\n";
                $dvStatus = udpRequest($dvRow[$service_port], $dvRow[$port_address], $dvIn);
                $dvStr .=  $dvRow[$service_port]." Status: ".$dvStatus."<br />";
                sleep(1);
                $dvIn = $getChipCount."\n";
                $dvChipCount = udpRequest($dvRow[$service_port], $dvRow[$port_address], $dvIn);
                $dvStr .=  $dvRow[$service_port]." Chip Count: ".$dvChipCount."<br />";
                for($dispCnt = 0; $dispCnt < $dvChipCount; $dispCnt++)
                {
                  $dvIn = $showChip.$dispCnt."\n";
                  $dvStatus = udpRequest($dvRow[$service_port], $dvRow[$port_address], $dvIn);
                  $dvStr .=  $dvRow[$service_port]." Chip info: ".$dvStatus."<br />";
                }
                echo $dvStr;
              }
            }
          ?>
        </td>
      </tr>
      <tr>
        <td  align="center" colspan="6">
          <table width="100%" border="1" cellspacing="0" cellpadding="2">
            <tr>
              <td align="center" colspan="6">
                <h2>Check For New Chips / Restore All Action Settings</h2>
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
                      <form method="post" action="CheckForNewDevices.php">
                        <input type="hidden" name="checknew" value="checknew">
                        <input type="submit" value="Scan For New Devices">
                      </form>
                      <font color="red">
                        USING THIS BUTTON TURNS OFF ALL SWITCHES AND CLEARS ALL ACTIONS!!
                        <br />
                        USE THIS BUTTON WITH EXTREME CARE!!
                        <br />              
                      </font>      
                    </td>
                    <td align="center">
                      <?php
                        $checkDBActiveStr = "SELECT * from `action` WHERE `tempAddr` != \"0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00\"";
                        $result = mysqli_query($link, $checkDBActiveStr);
                        $actionCnt = mysqli_num_rows($result);
                        $checkDBActiveStr = "SELECT * from `pid` WHERE `tempAddr` != \"0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00\"";
                        $result = mysqli_query($link, $checkDBActiveStr);
                        $pidCnt = mysqli_num_rows($result);
                        if(($pidCnt > 0) || ($actionCnt > 0))
                        {
                          $restoreAllStr = 
                            "
                              <form method=\"post\" action=\"ActionStatus.php\">
                                <input type=\"hidden\" name=\"restoreall\" value=\"restoreall\">
                                <input type=\"submit\" value=\"RESTORE ALL\">
                              </form>
                            ";
                        }else{
                          $restoreAllStr = 
                            "
                              <form method=\"post\" action=\"ActionStatus.php\">
                                <input type=\"hidden\" name=\"restoreall\" value=\"restoreall\">
                                <input type=\"submit\" value=\"RESTORE ALL\" disabled>
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
          <?php
            if(isset($_POST["dbclear"]) && $_POST["dbclear"] === "dbclear")
            {
              $dbClearStr = "UPDATE `action` SET `active`='0', `tempAddr`='0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00', `tcAddr`='0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00', `tcTrigger`='-255', `tcDelay`='0', `thAddr`='0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00', `thTrigger`='255', `thDelay`='0', `lcd`='0', `rgb`='0' WHERE 1";
              $result = mysqli_query($link, $dbClearStr);
              $dbClearStr = "UPDATE `chipNames` SET `address`='0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00',`name`='UNASSIGNED' WHERE 1";
              $result = mysqli_query($link, $dbClearStr);
              $dbClearStr = "UPDATE `pid` SET `enabled`='0', `tempAddr`='0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00', `setpoint`='70', `switchAddr`='0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00', `kp`='0', `ki`='0', `kd`='0', `direction`='0', `windowSize`='5000' WHERE 1";
              $result = mysqli_query($link, $dbClearStr);
              $dbClearStr = "DELETE FROM `actionGraph` WHERE 1";
              $result = mysqli_query($link, $dbClearStr);
              $dbClearStr = "DELETE FROM `pidGraph` WHERE 1";
              $result = mysqli_query($link, $dbClearStr);
              
            }
          ?>
          <font color="red">
            USE THIS BUTTON WITH EXTREME CARE!!
            <br />              
            USING THIS BUTTON RESETS ALL DATABASE VALUES!!
            <br />
          </font>
          <form method="post" action="CheckForNewRestore.php">
            <input type="hidden" name="dbclear" value="dbclear">
            <input type="submit" value="RESET DATABASE VALUES">
          </form>
          <font color="red">
            USING THIS BUTTON RESETS ALL DATABASE VALUES!!
            <br />
            USE THIS BUTTON WITH EXTREME CARE!!
            <br />
          </font>              
        </td>
      </tr>
      <tr>
        <td>
          <div id="container">
            <?php 
              // include ("updateStatus.php"); 
            ?>
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
