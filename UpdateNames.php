<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<?php
  include_once("udpRequest.php");
  include_once("accessDatabase.php");
  include_once("header.html");

  $chipAddrIndex    = 0;
  $chipIdIndex      = 0;
  $chipDataIndex    = 1;
  $chipNameIndex    = 2;
 
  $tempAddrStr = "";
  $switchAddrStr = ""; 

  if( (isset($_POST["netID"]) && $_POST["netID"] >= 0) || (isset($_GET["netID"]) && $_GET["netID"] >= 0) )
  {
    if( (isset($_POST["netID"]) && $_POST["netID"] >= 0) )
    {
      $netID = $_POST["netID"];
      $service_port = $_POST["service_port"];
      $port_address = $_POST["port_address"];
      $netName = $_POST["netName"];
    }else{
      $netID = $_GET["netID"];
      $service_port = $_GET["service_port"];
      $port_address = $_GET["port_address"];
      $netName = $_GET["netName"];
    }
    $headStr = "
      <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">
      <title> Sensor /Switch Setup </title>
      <link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"/>
      <script type=\"text/javascript\" src=\"//ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js\">
      </script>
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

  $h2Header = "<font color=\"blue\">$netName<br />Update Names</font>";
  
  if(isset($_POST["update"]) && $_POST["update"] === "update")
  {
    $h2Header="<font color=\"red\">".$netName." Names Updated</font>";
    $totalUpdated = 0;
    $maxChipCnt = $_POST["maxChipCnt"];
    $debugStr .= "\$maxChipCnt =  $maxChipCnt<br />";
    for($updateCnt=0; $updateCnt<$_POST["maxChipCnt"]; $updateCnt++)
    {
      $chipAddress = $_POST["address$updateCnt"];
      $chipNameStr = $_POST["name$updateCnt"];
      $query = "SELECT * from `chipNames` where `address`='".$chipAddress."' AND `netID`='".$netID."'";
//      echo $query."<br />";
      $result = mysqli_query($link,$query);

/*
      if($result === FALSE)
      {
        echo "name query failed<br />";
      }else{
        echo "name success<br />";
      }
*/
      $resultCnt = mysqli_num_rows($result);
//      echo "\$resultCnt = ".$resultCnt."<br />";
      mysqli_free_result($result);
      $chipNameStr = str_replace(" ","_",$chipNameStr);
      $chipNameStr = str_replace(",","_",$chipNameStr);
      $chipNameStr = str_replace(";","_",$chipNameStr);
      $escapedName = mysqli_real_escape_string ($link , $chipNameStr);
      $debugStr .= "\$escapedName = $escapedName <br />";
      if($resultCnt > 0)
      {
        $query = "UPDATE `chipNames` SET `id`='".$updateCnt."',`name`='".$escapedName."' WHERE `address`='".$chipAddress."' AND `netID`='".$netID."'";
      }else{
        $query = "INSERT INTO `chipNames` SET `id`='".$updateCnt."',`netID`='".$netID."',`address`='".$chipAddress."',`name`='".$escapedName."'";
      }
      $trimmedEscapedName = trim($escapedName);
      $debugStr .= "\$escapedName = $trimmedEscapedName <br />";
      
      if($trimmedEscapedName !== "_____UNASSIGNED_____")
      {
        $debugStr .= "\$query = $query<br />";

        $result = mysqli_query($link,$query);

        if($result === FALSE)
        {
          $debugStr .= "name update failed<br />";
        }else{
          $debugStr .= "name update success<br />";
        }
        $in = $updateChipName." ".$chipAddress." ".$chipNameStr;
        $result = udpRequest($service_port, $port_address, $in);
        $debugStr .= "\$result = $result<br />";
        $totalUpdated++;
      }
    }
    if($totalUpdated > 0)
    {
      $in = $saveToEEPROM;
      $result = udpRequest($service_port, $port_address, $in);
      $debugStr .= "$totalUpdated chips updated to EEPROM<br />";
      $h2Header="<font color=\"red\">$netName<br />$totalUpdated Names Updated</font>";
    }else{
      $debugStr .= "No chips updated to EEPROM<br />";
      $h2Header="<font color=\"red\">$netName<br />No Names Updated</font>";
    }
  }
  echo $headStr;
?>
  <head>
  </head>
  <body>
<!-- Table for Main Body -->
    <table width="100%" border="2" cellspacing="0" cellpadding="2">
      <tr>
        <td>
          <?php 
            include ("menu.php");
            echo "<br />
                  <!-- $debugStr -->
              </td>
            </tr>
            <tr>
              <td align=\"center\" colspan=\"2\" border=\"2\">
                <h2>".$h2Header."</h2>
              </td>
              </tr>
              <tr>
                <td align=\"center\">";
            
            $in = "$getChipCount\n";
            $out = udpRequest($service_port, $port_address, $in);

            $maxChipCnt = trim($out);
            echo "<br />
                  <form method=\"post\" action=\"UpdateNames.php\">
                    <input type=\"hidden\" name=\"update\" value=\"update\">
                    <input type=\"hidden\" name=\"maxChipCnt\" value=\"".$maxChipCnt."\">
                    <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
                    <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">
                    <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
                    <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
                    <table border=\"2\" cellpadding=\"2\" cellspacing=\"2\">
                    <tr>
                      <td align=\"center\" colspan=\"2\"><font size=\"5\"><strong>There are ".$out."
                        arrays that have a chip installed.</strong></font></td>
                    </tr>
                    <tr>
                      <td colspan=\"2\"align=\"center\">
                        <font size=\"5\"><strong>
                          Make your changes and press \"SUBMIT\"
                        </strong></font> 
                        <br /> 
                        <input type=\"submit\" value=\"SUBMIT\">
                      </td>
                    </tr>";
            for($scCnt=0; $scCnt < $maxChipCnt; $scCnt++)
            {
              $in = $showChip.$scCnt."\n";
              $out = udpRequest($service_port, $port_address, $in);
              $chipArray = explode(" ", $out);
              $chipAddressArray = explode(",", $chipArray[$chipAddrIndex]);
//              echo "chipAddress = $chipArray[$chipAddrIndex]<br />";
              $chipID = $chipAddressArray[$chipIdIndex];
              $chipName = $chipArray[$chipNameIndex];
//              echo "chipName = $chipName<br />";
              if(trim($chipName) === "_____UNASSIGNED_____")
              {
                $query = "SELECT name FROM chipNames WHERE `address`='$chipArray[$chipAddrIndex]' AND `netID`='$netID'";
//                echo "\$query = $query<br />";
                $result = mysqli_query($link, $query);
                $rowCnt = mysqli_num_rows($result);
//                echo "number of rows returned = $rowCnt<br />";
                if(($result !== FALSE) && ($rowCnt > 0))
                {
//                    echo "name query success, name = ";
                  $resultObj = mysqli_fetch_object($result);
                  $chipName = $resultObj->name;
//                    echo $resultObj->name."<br />";
                }else{
//                    echo "name query failed<br />";
                }
              }
                
                mysqli_free_result($result);
              
              if( ($chipID === "0x28") || 
                  ($chipID === "0x30") || 
                  ($chipID === "0xAA")
                )
              {
                $tempAddrStr .=  "
                  <tr>
                    <td align=\"center\">
                      ".$chipArray[$chipAddrIndex]."
                      <input type=\"hidden\" name=\"address".$scCnt."\" value=\"".$chipArray[$chipAddrIndex]."\">
                    </td>
                    <td align=\"center\">
                      <input type=\"text\" size=\"25\" name=\"name".$scCnt."\" value=\"".$chipName."\">
                    </td>
                  </tr>";
              }elseif($chipAddressArray[$chipIdIndex] == "0x12"){
                $switchAddrStr .=  "  <tr>
                  <td align=\"center\">
                    ".$chipArray[$chipAddrIndex]."
                     <input type=\"hidden\" name=\"address".$scCnt."\" value=\"".$chipArray[$chipAddrIndex]."\">
                  </td>
                  <td align=\"center\">
                    <input type=\"text\" size=\"25\" name=\"name".$scCnt."\" value=\"".$chipName."\">
                  </td>
                </tr>";
              }
            }
            echo "<tr>
                    <td>
                      <table>
                        <tr>";
            if($tempAddrStr != "")
            {            
              echo"
                <td>
                  <table border=\"2\">
                    <tr>
                      <td align=\"center\"><font size=\"5\"><strong>Temperature Address</strong></font></td>
                      <td align=\"center\"><font size=\"5\"><strong>Name</strong></font></td>
                    </tr>
                    ".$tempAddrStr."
                  </table>  
                </td>";
              }
              if($switchAddrStr != "")
              {            
                echo"
                  <td>
                    <table border=\"2\">
                      <tr>
                        <td align=\"center\"><font size=\"5\"><strong>Switch Address</strong></font></td>
                        <td align=\"center\"><font size=\"5\"><strong>Name</strong></font></td>
                      </tr>
                       ".$switchAddrStr."
                     </table>
                   </td>";
              }
              echo"
                </tr>
              </table>
            </td>
          </tr/>
          <tr>
            <td colspan=\"2\"align=\"center\">
              <font size=\"5\"><strong>
                Make your changes and press \"SUBMIT\" 
              </strong></font> 
              <br /> 
              <input type=\"submit\" value=\"SUBMIT\">
            </td>
          </tr>
        </table>
      </form";
          ?>
        </td>
      </tr>
      <tr>
        <td>
          <br />
          <?php
            include ("menu.php");
          ?>
        </td>
      </tr>
    </table>
  </body>
</html>

