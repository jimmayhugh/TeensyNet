<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<?php
  include_once("udpRequest.php");
  include_once("accessDatabase.php");
  include_once("header.html");

  $glcdNameIndex    =  0;
  $glcdFlagIndex    =  1;
  $glcdAction0Index =  2;
  $glcdAction1Index =  3;
  $glcdAction2Index =  4;
  $glcdAction3Index =  5;
  $glcdTemp0Index   =  6;
  $glcdTemp1Index   =  7;
  $glcdTemp2Index   =  8;
  $glcdTemp3Index   =  9;
  $glcdTemp4Index   = 10;
  $glcdTemp5Index   = 11;
  $glcdTemp6Index   = 12;
  $glcdTemp7Index   = 13;
  $glcdAddrIndex    = 14;
  
  $glcdIdIndex = 0;
 
  $glcdAddrStr = "";
  $maxGLCDCnt = 0;
  
  $lcdAddrIndex  =  0;
  $lcdTypeIndex  =  1;
  $lcdNameIndex  =  2;
  $lcdIdIndex    =  0;
  $lcdAddrStr    = "";
  
  $tempAddrStr = "";
  $result = "";
  $debugStr = "";

  $dsLCD          = "0x47"; // Teensy 3.x 1-wire slave 4x20 HD44780 LCD
  $dsGLCDP        = "0x45"; // Teensy 3.1 1-wire slave 800x400 7" GLCD with Paging
  $dsGLCD         = "0x44"; // Teensy 3.1 1-wire slave 800x400 7" GLCD

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
      <meta http-equiv=\"refresh\" content=\"1;url=http://localhost/index.php\">
      <script type=\"text/javascript\">
          window.location.href = \"http://localhost/index.php\"
      </script>";
  }

  $h2Header = "<font color=\"blue\">$netName<br />Update 1-Wire&reg; Display Names</font>";
  
  if(isset($_POST["devupdate"]) && $_POST["devupdate"] === "devupdate")
  {
    $in = $updateBonjour.$netName."\n";
    $out = udpRequest($service_port, $port_address, $in);
    $devUpdateStr = "UPDATE `netDevices` SET `netName`='$netName' WHERE `service_port`='$service_port'";
    $devResult = mysqli_query($link, $devUpdateStr);
    mysqli_free_result($devResult);
  }
  
  if(isset($_POST["updateGLCD"]) && $_POST["updateGLCD"] === "updateGLCD")
  {
    $h2Header="<font color=\"red\">".$netName." Names Updated</font>";
    $totalUpdated = 0;
    $maxGLCDCnt = $_POST["maxGLCDCnt"];
    $debugStr .= "\$maxGLCDCnt =  $maxGLCDCnt<br />";
    for($updateCnt=0; $updateCnt<$_POST["maxGLCDCnt"]; $updateCnt++)
    {
      $glcdAddress = $_POST["glcdAddress$updateCnt"];
      $glcdNameStr = $_POST["glcdName$updateCnt"];
      $query = "SELECT * from `glcdNames` where `address`='".$glcdAddress."' AND `netID`='".$netID."'";
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
      $glcdNameStr = str_replace(" ","_",$glcdNameStr);
      $glcdNameStr = str_replace(",","_",$glcdNameStr);
      $glcdNameStr = str_replace(";","_",$glcdNameStr);
      $escapedName = mysqli_real_escape_string ($link , $glcdNameStr);
      $debugStr .= "\$escapedName = $escapedName <br />";
      if($resultCnt > 0)
      {
        $query = "UPDATE `glcdNames` SET `id`='".$updateCnt."',`name`='".$escapedName."' WHERE `address`='".$glcdAddress."' AND `netID`='".$netID."'";
      }else{
        $query = "INSERT INTO `glcdNames` SET `id`='".$updateCnt."',`netID`='".$netID."',`address`='".$glcdAddress."',`name`='".$escapedName."'";
      }
      $trimmedEscapedName = trim($escapedName);
      $debugStr .= "\$escapedName = $trimmedEscapedName <br />";
      
      if($trimmedEscapedName !== "NULL")
      {
        $debugStr .= "\$query = $query<br />";

        $result = mysqli_query($link,$query);

        if($result === FALSE)
        {
          $debugStr .= "name update failed<br />";
        }else{
          $debugStr .= "name update success<br />";
        }
        $in = $updateglcd1wName." ".$glcdAddress." ".$glcdNameStr;
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
      $h2Header="<font color=\"red\">$netName<br />$totalUpdated GLCD Names Updated</font>";
    }else{
      $debugStr .= "No chips updated to EEPROM<br />";
      $h2Header="<font color=\"red\">$netName<br />No Names Updated</font>";
    }
  }

  if(isset($_POST["updateLCD"]) && $_POST["updateLCD"] === "updateLCD")
  {
    echo "Updating LCD names<br />";
    $h2Header="<font color=\"red\">".$netName." Names Updated</font>";
    $totalUpdated = 0;
    $maxLCDCnt = $_POST["maxLCDCnt"];
    $debugStr .= "\$maxLCDCnt =  $maxLCDCnt<br />";
    for($updateCnt=0; $updateCnt<$_POST["maxLCDCnt"]; $updateCnt++)
    {
      $lcdAddress = $_POST["lcdAddress$updateCnt"];
      $lcdNameStr = $_POST["lcdName$updateCnt"];
      $query = "SELECT * from `lcdNames` where `address`='".$lcdAddress."' AND `netID`='".$netID."'";
      echo $query."<br />";
      $result = mysqli_query($link,$query);


      if($result === FALSE)
      {
        echo "name query failed<br />";
      }else{
        echo "name success<br />";
      }

      $resultCnt = mysqli_num_rows($result);
      echo "\$resultCnt = ".$resultCnt."<br />";
      mysqli_free_result($result);
      $lcdNameStr = str_replace(" ","_",$lcdNameStr);
      $lcdNameStr = str_replace(",","_",$lcdNameStr);
      $lcdNameStr = str_replace(";","_",$lcdNameStr);
      $escapedName = mysqli_real_escape_string ($link , $lcdNameStr);
      $debugStr .= "\$escapedName = $escapedName <br />";
      if($resultCnt > 0)
      {
        $query = "UPDATE `lcdNames` SET `id`='".$updateCnt."',`name`='".$escapedName."' WHERE `address`='".$lcdAddress."' AND `netID`='".$netID."'";
      }else{
        $query = "INSERT INTO `lcdNames` SET `id`='".$updateCnt."',`netID`='".$netID."',`address`='".$lcdAddress."',`name`='".$escapedName."'";
      }
      $trimmedEscapedName = trim($escapedName);
      $debugStr .= "\$escapedName = $trimmedEscapedName <br />";
      
      if($trimmedEscapedName !== "NULL")
      {
        $debugStr .= "\$query = $query<br />";

        $result = mysqli_query($link,$query);

        if($result === FALSE)
        {
          $debugStr .= "name update failed<br />";
        }else{
          $debugStr .= "name update success<br />";
        }
        $in = $updatelcd1wName." ".$lcdAddress." ".$lcdNameStr;
        $result = udpRequest($service_port, $port_address, $in);
        $debugStr .= "\$result = $result<br />";
        $totalUpdated++;
      }
    }
    if($totalUpdated > 0)
    {
      $in = $saveToEEPROM;
      $result = udpRequest($service_port, $port_address, $in);
      $debugStr .= "$totalUpdated displays updated to EEPROM<br />";
      $h2Header.="<font color=\"red\"><br />$totalUpdated LCD Names Updated</font>";
    }else{
      $debugStr .= "No displays updated to EEPROM<br />";
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
        <td colspan="4">
        <?php 
          include ("menu.php");
          echo "<br />
            <!-- $debugStr -->
        </td>
      </tr>
      <tr>
        <td align=\"center\" colspan=\"4\" border=\"2\">
          <h2>".$h2Header."</h2>
          <form method=\"post\" action=\"updateGLCDnames.php\">
            <input type=\"hidden\" name=\"devupdate\" value=\"devupdate\">
            <input type=\"hidden\" name=\"maxGLCDCnt\" value=\"".$maxGLCDCnt."\">
            <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
            <input type=\"text\" size=\"20\" maxlength=\"20\" name=\"netName\" value=\"".$netName."\">
            <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
            <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
            <br />
            <input type=\"submit\" value=\"UPDATE DEVICE NAME\">
          </form>
        </td>
      </tr>
      <tr>
        <td align=\"center\">";
            $in = "$getLCDcnt\n";
            $out = udpRequest($service_port, $port_address, $in);

            $maxLCDCnt = trim($out);
//            echo "<br /> maxLCDCnt = $maxLCDCnt <br />";
            $lcdFormStr="<br />
                  <form method=\"post\" action=\"updateGLCDnames.php\">
                    <input type=\"hidden\" name=\"updateLCD\" value=\"updateLCD\">
                    <input type=\"hidden\" name=\"maxLCDCnt\" value=\"".$maxLCDCnt."\">
                    <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
                    <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">
                    <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
                    <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">";
                    
                    "<table border=\"2\" cellpadding=\"2\" cellspacing=\"2\">
                    <tr>
                      <td align=\"center\" colspan=\"2\"><font size=\"5\"><strong>There are ".$out."
                        LCD(s) installed.</strong></font></td>
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
            for($scCnt=0; $scCnt < $maxLCDCnt; $scCnt++)
            {
              $in = $getLCDstatus.$scCnt."\n";
              $out = udpRequest($service_port, $port_address, $in);
              echo "out = $out<br />";
              $lcdArray = explode(" ", $out);
              $lcd1wAddrArray = explode(",", $lcdArray[$lcdAddrIndex]);
              $lcd1wAddress = $lcdArray[$lcdAddrIndex];
              $lcdType = $lcdArray[$lcdTypeIndex];
              $lcdName = $lcdArray[$lcdNameIndex];

              echo "\$lcd1wAddress = $lcd1wAddress<br />";
              echo "\$lcdType = $lcdType<br />";
              echo "\$lcdName = $lcdName<br />";

              $lcdID = $lcd1wAddrArray[$lcdIdIndex];
//              echo "lcdID = $lcdID<br />";
              $lcdName = $lcdArray[$lcdNameIndex];
//              echo "chipName = $lcdName<br />";
              if(trim($lcdName) === "__UNASSIGNED___")
              {
                $query = "SELECT name FROM `lcdNames` WHERE `address`='$lcd1wAddress' AND `netID`='$netID'";
//                $query = "SELECT name FROM lcdNames WHERE `address`='$lcdAddrArrayStr' AND `netID`='$netID'";
                echo "\$query = $query<br />";
                $result = mysqli_query($link, $query);
                $rowCnt = mysqli_num_rows($result);
                echo "number of rows returned = $rowCnt<br />";
                if(($result !== FALSE) && ($rowCnt > 0))
                {
                    echo "name query success, name = ";
                  $resultObj = mysqli_fetch_object($result);
                  $lcdName = $resultObj->name;
                    echo $resultObj->name."<br />";
                }else{
                    echo "name query failed<br />";
                }
              }
                
              mysqli_free_result($result);
              
              echo "\$lcdID = $lcdID, \$dsLCD = $dsLCD<br />"; 

              if(trim($lcdID) === trim($dsLCD))
              {
                echo "lcdID === dsLCD<br />";
                $lcdTempAddrStr .="
                  <tr>
                    <td align=\"center\">
                      <input type=\"hidden\" name=\"lcdAddress".$scCnt."\" value=\"".$lcd1wAddress."\">
                    </td>
                    <td align=\"center\">
                      <input type=\"text\" size=\"25\" name=\"lcdName".$scCnt."\" value=\"".$lcdName."\">
                    </td>
                  </tr>";
              }
              echo "lcdTempAddrStr = $lcdTempAddrStr<br />";
            }
            echo "<tr>
                    <td>
                      <table>
                        <tr>";
            if($lcdTempAddrStr != "")
            {            
              echo"
                <td>
                  <table border=\"2\">
                    <tr>
                      <td align=\"center\"><font size=\"5\"><strong>LCD Address</strong></font></td>
                      <td align=\"center\"><font size=\"5\"><strong>Name</strong></font></td>
                    </tr>
                    ".$lcdTempAddrStr."
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
      </form>
          
        </td>
          <td align=\"center\">";
            
            $in = "$getGLCDcnt\n";
            $out = udpRequest($service_port, $port_address, $in);

            $maxGLCDCnt = trim($out);
//            echo "<br /> maxGLCDCnt = $maxGLCDCnt <br />";
            echo "<br />
                  <form method=\"post\" action=\"updateGLCDnames.php\">
                    <input type=\"hidden\" name=\"updateGLCD\" value=\"updateGLCD\">
                    <input type=\"hidden\" name=\"maxGLCDCnt\" value=\"".$maxGLCDCnt."\">
                    <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
                    <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">
                    <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
                    <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
                    <table border=\"2\" cellpadding=\"2\" cellspacing=\"2\">
                    <tr>
                      <td align=\"center\" colspan=\"2\"><font size=\"5\"><strong>There are ".$out."
                        GLCD(s) installed.</strong></font></td>
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
            for($scCnt=0; $scCnt < $maxGLCDCnt; $scCnt++)
            {
              $in = $getGLCDstatus.$scCnt."\n";
              $out = udpRequest($service_port, $port_address, $in);
              $glcdArray = explode(",", $out);
              $glcdAddressArray = explode(";", $glcdArray[$glcdAddrIndex]);
              $glcdAddrArrayStr = "";
              for( $addrCnt = 0; $addrCnt < 8; $addrCnt++)
              {
                $glcdAddrArrayStr .= "0x$glcdAddressArray[$addrCnt]";
                if($addrCnt < 7)
                {
                  $glcdAddrArrayStr .= ",";
                }
              }
//              echo "chipAddress = $glcdAddrArrayStr<br />";
              $glcdID = $glcdAddressArray[$glcdIdIndex];
              $glcdName = $glcdArray[$glcdNameIndex];
//              echo "chipName = $glcdName<br />";
              if(trim($glcdName) === "NULL")
              {
                $query = "SELECT name FROM glcdNames WHERE `address`='$glcdArray[$glcdAddrIndex]' AND `netID`='$netID'";
//                echo "\$query = $query<br />";
                $result = mysqli_query($link, $query);
                $rowCnt = mysqli_num_rows($result);
//                echo "number of rows returned = $rowCnt<br />";
                if(($result !== FALSE) && ($rowCnt > 0))
                {
//                    echo "name query success, name = ";
                  $resultObj = mysqli_fetch_object($result);
                  $glcdName = $resultObj->name;
                    echo $resultObj->name."<br />";
                }else{
//                    echo "name query failed<br />";
                }
              }
                
                mysqli_free_result($result);
              
              if( trim($glcdID) === trim($dsGLCDP) )
              {
                $glcdTempAddrStr .=  "
                  <tr>
                    <td align=\"center\">
                      ".$glcdAddrArrayStr."
                      <input type=\"hidden\" name=\"glcsAddress".$scCnt."\" value=\"".$glcdAddrArrayStr."\">
                    </td>
                    <td align=\"center\">
                      <input type=\"text\" size=\"25\" name=\"glcdName".$scCnt."\" value=\"".$glcdName."\">
                    </td>
                  </tr>";
              }
            }
            echo "<tr>
                    <td>
                      <table>
                        <tr>";
            if($glcdTempAddrStr != "")
            {            
              echo"
                <td>
                  <table border=\"2\">
                    <tr>
                      <td align=\"center\"><font size=\"5\"><strong>GLCD Address</strong></font></td>
                      <td align=\"center\"><font size=\"5\"><strong>Name</strong></font></td>
                    </tr>
                    ".$glcdTempAddrStr."
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
        <td colspan="4">
          <br />
          <?php
            include ("menu.php");
          ?>
        </td>
      </tr>
    </table>
  </body>
</html>

