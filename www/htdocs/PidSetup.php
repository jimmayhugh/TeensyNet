<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<META HTTP-EQUIV="CACHE-CONTROL" CONTENT="NO-CACHE">
<?php
  include_once("accessDatabase.php");
  include_once("udpRequest.php");
  if(isset($_GET["netName"])) 
  {
    $service_port=$_GET["service_port"];
    $port_address=$_GET["port_address"];
    $netName=$_GET["netName"];
    $netID=$_GET["netID"];
  //      echo "PidSetup.php: netID = $netID, service_port = $service_port, port_address = $port_address, netName = $netName <br />";
  }

  if(isset($_POST["netName"])) 
  {
    $service_port=$_POST["service_port"];
    $port_address=$_POST["port_address"];
    $netName=$_POST["netName"];
    $netID=$_POST["netID"];
  //      echo "PidSetup.php: netID = $netID, service_port = $service_port, port_address = $port_address, netName = $netName <br />";
  }

  $pidCnt = $_POST["pidCnt"];
  echo "<title> PID#".$pidCnt." Data </title>";
?>
<link rel="stylesheet" type="text/css" href="style.css"/>
<script type="text/javascript" src="//ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js"></script>
<!-- <script type="text/javascript" src="js/jquery.js"></script> -->
<style>
input[type='text'] { font-size: 25px; text-align: center;}
input:focus, textarea:focus{background-color: lightgrey;}
select[type='text'] { font-size: 25px; text-align: center;}
option[type='text'] { font-size: 25px; text-align: center;}
</style>
</head>
  <body>
    <?php 
      include ("header.html");
      $bodyStr = "";
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
          <div id="container">
            <h2> <font color = "blue">PID Setup ID# <?php echo $pidCnt; ?></font><h2>
            <?php
              $pidIn = $getPidArray.$pidCnt."\n";
//              echo "pidIn = ".$pidIn."<br />";
              $pidOut = udpRequest($service_port, $port_address, $pidIn);
//              echo "pidOut = ".$pidOut."<br />";
              if($pidOut === "Z")
              {
                echo "Invalid Return<br />";
              }else{
                $pidArray = explode(" ", $pidOut);
                
                $pidEnabledStr      = trim($pidArray[0]);
                $pidTempAddrStr     = trim($pidArray[1]);
                $pidTempNameStr     = trim($pidArray[2]);
                $pidTempStatusStr   = trim($pidArray[3]);
                $pidSetPointStr     = trim($pidArray[4]);
                $pidSwitchAddrStr   = trim($pidArray[5]);
                $pidSwitchNameStr   = trim($pidArray[6]);
                $pidSwitchStatusStr = trim($pidArray[7]);
                $pidKpStr           = trim($pidArray[8]);
                $pidKiStr           = trim($pidArray[9]);
                $pidKdStr           = trim($pidArray[10]);
                $pidDirectionStr    = trim($pidArray[11]);
                $pidWindowSizeStr   = trim($pidArray[12]);

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
                   echo $bodyStr;    
*/


                $query = "select * from chipNames where netID='".$netID."'";
                $result = mysqli_query($link,$query);
                if($result)
                {
                  $pidObj = mysqli_fetch_object($result);
                  $pidTempName = $pidObj->name;
                }else{
                  $pidTempName = "UNASSIGNED";
                }
                mysqli_free_result($result);
                
                $query = "SELECT * FROM chipNames WHERE netID='".$netID."'";
                $result = mysqli_query($link,$query);
                if($result)
                {
                  $pidObj = mysqli_fetch_object($result);
                  $pidSwitchName = $pidObj->name;
                }else{
                  $pidSwitchName = "UNASSIGNED";
                }
                mysqli_free_result($result);
              }
              
            ?>
          </div>
        </td>
      </tr>
      <tr>
            <?php
              $bodyStr .= 
              "<div id=\"pid".$pidCnt."\">
                <td valign=\"top\" align=\"center\">
                  <table border=\"1\" width=\"50%\" cellspacing=\"0\" cellpadding=\"10\">
                    <tr>
                      <td align=\"center\" colspan=\"4\">
                        <div style=\"vertical-align:middle; min-height:50px;\">
                          <font size=\"5\" color=\"blue\"><strong>
                           PID# ".$pidCnt." Setup
                          </strong></font>
                        <form name=\"pidSetup\" method=\"post\" action=\"PidStatus.php\">
                          <input type=\"hidden\" name=\"pidSetup\" value=\"pidSetup\">
                          <input type=\"hidden\" name=\"pidCnt\" value=\"".$pidCnt."\">
                          <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
                          <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
                          <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">
                          <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
                          <input  style=\" font-size : 25px\" type=\"submit\" value=\"MODIFY\">
                      </td>
                    </tr>
                    <tr>
                      <td align=\"center\" colspan=\"4\">";
              if($pidEnabledStr === "0")
              {
                $bodyStr .= 
                   "
                      <input type=\"radio\" name=\"pidEnable\" value=\"1\"><font size=\"5\">Enable</font></input>
                      <input type=\"radio\" name=\"pidEnable\" value=\"0\" checked><font size=\"5\" color=\"red\"><strong>DISABLED</strong></font></input>
                    ";
              }else if($pidEnabledStr === "1"){
                $bodyStr .= 
                  "
                      <input type=\"radio\" name=\"pidEnable\" value=\"1\" checked><font size=\"5\" color=\"green\"><strong>ENABLED</strong></font></input>
                      <input type=\"radio\" name=\"pidEnable\" value=\"0\"><font size=\"5\">Disable</font></input>
                  ";
              }
              
              $bodyStr .= 
                 "</td>
               </tr>
               <tr>
                <td align=\"center\" colspan=\"4\">
                  <div style=\"vertical-align:middle; min-height:50px;\">
                    <font size=\"5\" color=\"blue\"><strong>
                      Temperature Chip
                      <select style=\" font-size : 25px\" name=\"pidTempAddr\">";
                        $addrStrArray = explode(",", $pidTempAddrStr);
                        if(($pidTempAddrStr !== $dummyAddr) && ($addrStrArray[0] === "0x28") )
                        {
                          $bodyStr .=
                          "<option value=\"".$pidTempAddrStr."\" selected>".$pidTempNameStr."</option>
                           <option value=\"".$dummyAddr."\">UNASSIGNED</option>";
                        }else{
                          $bodyStr .=
                          "<option value=\"".$dummyAddr."\">UNASSIGNED</option>";
                        }
                      $query = "SELECT address, name FROM chipNames WHERE netID=".$netID;
                      $result = mysqli_query($link,$query);
                      while($pidObj = mysqli_fetch_object($result))
                      {
                        $addrStrArray = explode(",", $pidObj->address);
                        if($addrStrArray[0] === "0x28")
                        {
                          $bodyStr .= "<option value=\"".$pidObj->address."\">".$pidObj->name."</option>";
                        }
                      }
                      mysqli_free_result($result);
              $bodyStr .= 
                 "
                   </select>
                    </strong></font>
                  </div>
                </td>
               </tr>
               <tr>
                <td align=\"center\" colspan=\"4\">
                  <div style=\"vertical-align:middle; min-height:50px;\">
                    <font size=\"5\" color=\"blue\"><strong>
                      Set Point
                      <input type=\"text\" name=\"pidSetPoint\" size=\"10\" maxlength=\"10\" value=\"".$pidSetPointStr."\"></input>
                    </strong></font>
                  </div>
                </td>
               </tr>
               <tr>
                <td align=\"center\" colspan=\"4\">
                  <div style=\"vertical-align:middle; min-height:50px;\">
                    <font size=\"5\" color=\"blue\"><strong>
                      Switch Chip
                      <select style=\" font-size : 25px\" name=\"pidSwitchAddr\">";
                        $addrStrArray = explode(",", $pidSwitchAddrStr);
                        if(($pidSwitchAddrStr !== $dummyAddr) && ($addrStrArray[0] === "0x12") )
                        {
                          $bodyStr .=
                          "<option value=\"".$pidSwitchAddrStr."\" selected>".$pidSwitchNameStr."</option>
                           <option value=\"".$dummyAddr."\">UNASSIGNED</option>";
                        }else{
                          $bodyStr .=
                          "<option value=\"".$dummyAddr."\">UNASSIGNED</option>";
                        }
                      $query = "SELECT address, name FROM chipNames WHERE netID=".$netID;
                      $result = mysqli_query($link,$query);
                      while($pidObj = mysqli_fetch_object($result))
                      {
                        $addrStrArray = explode(",", $pidObj->address);
                        if($addrStrArray[0] === "0x12")
                        {
                          $bodyStr .= "<option value=\"".$pidObj->address."\">".$pidObj->name."</option>";
                        }
                      }
                      mysqli_free_result($result);
              $bodyStr .= 
                 "
                   </select>
                    </strong></font>
                  </div>
                </td>
                  ";
                
              $bodyStr .= "
              </tr>
              <tr>
                <td align=\"center\" colspan=\"4\">
                  <div style=\"vertical-align:middle; min-height:50px;\">
                    <font size=\"5\" color=\"blue\"><strong>PID Variables<br />
                      Kp:&nbsp;<input type=\"text\" size=\"10\" maxlength=\"10\" name=\"pidKp\" value=\"".$pidKpStr."\"></input><br />
                      Ki:&nbsp;<input type=\"text\" size=\"10\" maxlength=\"10\" name=\"pidKi\" value=\"".$pidKiStr."\"></input><br />
                      Kd:&nbsp;<input type=\"text\" size=\"10\" maxlength=\"10\" name=\"pidKd\" value=\"".$pidKdStr."\"></input>
                    </strong></font>
                  </div>
              ";
              
              $bodyStr .= "
                </td>
              </tr>
              <tr>  
                <td align=\"center\" colspan=\"4\">
                <font size=\"5\" color=\"blue\"><strong>
                  Direction<br />
                </strong></font>";
              if($pidDirectionStr === "0")
              {
                $bodyStr .= 
                   "
                      <input type=\"radio\" name=\"pidDirection\" value=\"0\" checked><font size=\"5\" color=\"red\"><strong>FORWARD</strong></font></input>
                      <input type=\"radio\" name=\"pidDirection\" value=\"1\"><font size=\"5\" >Reverse</font></input>
                    ";
              }else if($pidDirectionStr === "1"){
                $bodyStr .= 
                  "
                      <input type=\"radio\" name=\"pidDirection\" value=\"0\"><font size=\"5\">Forward</font></input>
                      <input type=\"radio\" name=\"pidDirection\" value=\"1\" checked><font size=\"5\" color=\"blue\"><strong>REVERSE</strong></font></input>
                  ";
              }
              
              $bodyStr .="</div>";
              $bodyStr .= "
                </td>
              </tr>
              <tr>
                <td align=\"center\" colspan=\"4\">
                  <div style=\"vertical-align:middle; min-height:50px;\">
                    <font size=\"5\" color=\"blue\"><strong>PID Window Size (Milliseconds)<br />
                      <input type=\"text\" size=\"6\" maxlength=\"6\" name=\"pidWindowSize\"value=\"".$pidWindowSizeStr."\"></input>
                    </font>
                    </strong></font>
                  </div>";
              $bodyStr .= "
                      </td>
                    </tr>
                    <tr>
                      <td align=\"center\" colspan=\"4\">
                        <div style=\"vertical-align:middle; min-height:50px;\">
                            <input type=\"hidden\" name=\"pidCnt\" value=\"".$pidCnt."\">
                            <input  style=\" font-size : 25px\" type=\"submit\" value=\"MODIFY\">
                          </form><br />
                          <font size=\"5\" color=\"blue\"><strong>
                           PID# ".$pidCnt." Setup
                          </strong></font>
                      </td>
                    </tr>
                  </table>
                </td>
              </div>";
              echo $bodyStr;
            ?>
        </td>
      <tr>
        <td valign="top" align="left" width="150">
        <?php 
        include ("menu.php");
        ?>
        </td>
      </tr>
    </table>
    <?php
      mysqli_close($link);
    ?>
  </body>
</html>
 
