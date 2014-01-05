<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title> Sensor /Switch Update </title>
<link rel="stylesheet" type="text/css" href="style.css"/>
<script type="text/javascript" src="//ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js"></script>
<!-- <script type="text/javascript" src="js/jquery.js"></script> -->
</head>
<body>
<?php
  include("header.html");
?> 
<!-- Table for Main Body -->
<table width="100%" border="0" cellspacing="0" cellpadding="2">
  <tr>
    <td>
    <?php 
      include_once("accessDatabase.php");
      include_once("udpRequest.php");
    ?>
    </td>
  </tr>
  <tr>
    <td  align="center" colspan="6">
      <table width="100%" border="1" cellspacing="0" cellpadding="2">
        <tr>
          <td align="center" border="2" colspan="6">
            <?php 
              $netID      = 0;
              $netName    = $netID+1;
              $service_port = $netName+1;
              $port_address    = $service_port+1;
              $netActive  = $port_address+1;
              
              $dvNetworkDevicesStr = "SELECT * FROM `netDevices` WHERE netActive=1";
              $dvResult = mysqli_query($link, $dvNetworkDevicesStr);
              $dvCnt = mysqli_num_rows($dvResult);
              echo "<h2>Select a TeensyNet Device<br /><font color =\"red\">".$dvCnt." TeensyNet Devices Found</font></h2>";
              $formStr = "<form method=\"post\" action=\"index.php\">
                <input type=\"hidden\" name=\"checknew\" value=\"checknew\">
                <input type=\"submit\" value=\"Scan For New Devices\">
              </form>";
              echo "<center>".$formStr."</center>";
            ?>
          </td>
        </tr>
        </table>      
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
      $pageArray = [
                    ["Sensor Status", "SensorStatus.php"],
                    ["Action Status", "ActionStatus.php"],
                    ["PID Status" ,"PidStatus.php"],
                    ["Check For New / Restore" , "CheckForNewRestore.php"],
                    ["Update Names", "UpdateNames.php"]
                   ];
      $formStr = "<table width=\"100%\" border=\"1\" cellspacing=\"0\" cellpadding=\"2\"><tr>";
      while($dvRow = mysqli_fetch_array($dvResult))
      {
        $dvIn = $getChipCount."\n";
        $dvChipCount = udpRequest($dvRow[$service_port], $dvRow[$port_address], $dvIn);
        $dvIn = $versionID."\n";
        $dvVersionID = udpRequest($dvRow[$service_port], $dvRow[$port_address], $dvIn);
        $formStr .= 
          "<td align = \"center\"><h2><font color=\"blue\">".$dvRow[$netName]."</font><br /><h2>Chip Count: ".$dvChipCount."</h2>";
        foreach($pageArray as $pageData)
        {
          $formStr .= 
          "<form method=\"post\" action=\"".$pageData[1]."\">\n";
          if($pageData[0] === "Update Names")
          {
            $formStr .= 
            "<input type=\"hidden\" name=\"stopUpdate\" value=\"1\">\n";
          }else{
            $formStr .= 
            "<input type=\"hidden\" name=\"stopUpdate\" value=\"0\">\n";
          }
          $formStr .= 
            "<input type=\"hidden\" name=\"netID\" value=\"".$dvRow[$netID]."\">
            <input type=\"hidden\" name=\"netName\" value=\"".$dvRow[$netName]."\">
            <input type=\"hidden\" name=\"service_port\" value=\"".$dvRow[$service_port]."\">
            <input type=\"hidden\" name=\"port_address\" value=\"".$dvRow[$port_address]."\">
            <input type=\"submit\" value=\"".$pageData[0]."\" style=\"height:35px; width:300px\">
          </form><br />";
        }
        $formStr .= "<br />".$dvVersionID."</center></td>";
      }
      $formStr .= "</tr></table>";
      echo $formStr;
    ?>
    </td>
  </tr>
</table>
</body></html>
