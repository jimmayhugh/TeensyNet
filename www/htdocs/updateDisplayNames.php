<?php
//  $debugFile = fopen("/home/jimm/debug/debug.txt", "a+");
//  $now = date("Y/m/d - h:i:sa");
//  fwrite($debugFile, $now."\n");
  include_once("udpRequest.php");
  include_once("accessDatabase.php");
  include_once("headStr.php");
  include_once("logoStr.php");
  include_once("menuStr.php");
  include_once("footStr.php");
  include_once("glcdDefines.php");

// 4x20 LCD defines
  $lcdAddrIndex =  0;
  $lcdTypeIndex =  1;
  $lcdNameIndex =  2;
  $lcdIdIndex   =  0;
  $lcdAddrStr   = "";
  $maxLCDcnt    =  0;
  
// 1-wire family codes
  $dsLCD          = "0x47"; // Teensy 3.x 1-wire slave 4x20 HD44780 LCD
  $dsGLCDP        = "0x45"; // Teensy 3.1 1-wire slave 800x400 7" GLCD with Paging
  $dsGLCD         = "0x44"; // Teensy 3.1 1-wire slave 800x400 7" GLCD

// common defines
  $tempAddrStr = "";
  $result = "";
  $debugStr = "
    <table width=\"100%\" border=\"1\" cellspacing=\"0\" cellpadding=\"2\">
      <tr>
        <td>
";
  $h2Header = "<h2><font color=\"blue\">$netName<br />Update 1-Wire&reg; Display Names</font>";

  $formStr="
    <table width=\"100%\" border=\"1\" cellspacing=\"0\" cellpadding=\"2\">
      <tr>
        <td align=\"center\" colspan=\"4\">
          <form method=\"post\" action=\"updateDisplayNames.php\">
            <input type=\"hidden\" name=\"update\" value=\"update\">
            <input type=\"submit\" value=\"UPDATE NAMES\">
            <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
            <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
            <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
         </td>
      <tr>
  ";

// if arriving here from a POST or GET request, act accordingly. Otherwise return to index page
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
  }else{
    $headStr = "
      <meta charset=\"UTF-8\">
      <meta http-equiv=\"refresh\" content=\"1;url=http://localhost/index.php\">
      <script type=\"text/javascript\">
          window.location.href = \"http://localhost/index.php\"
      </script>";
  }


  if(isset($_POST["update"]) && $_POST["update"] === "update")
  {
    $totalUpdated = 0;

// update the GLCD names
    $maxGLCDcnt = $_POST["maxGLCDcnt"];
    $debugStr .= "\$maxGLCDcnt =  $maxGLCDcnt<br />";
    for($updateCnt=0; $updateCnt<$_POST["maxGLCDcnt"]; $updateCnt++)
    {
      $glcdAddress = $_POST["glcdAddress$updateCnt"];
      $glcdNameStr = $_POST["glcdName$updateCnt"];
      $query = "SELECT * from `glcdNames` where `address`='".trim($glcdAddress)."' AND `netID`='".$netID."'";
      $debugStr .= $query."<br />";
      $result = mysqli_query($link,$query);


      if($result === FALSE)
      {
        $debugStr .="GLCD name query failed<br />";
      }else{
        $debugStr .="GLCD name success<br />";
      }

      $resultCnt = mysqli_num_rows($result);
      $debugStr .= "\$resultCnt = ".$resultCnt."<br />";
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
          $debugStr .= "GLCD name update failed<br />";
        }else{
          $debugStr .= "GLCD name update success<br />";
        }
        $in = $updateGLCD1wName." ".$glcdAddress." ".$glcdNameStr;
        $result = udpRequest($service_port, $port_address, $in);
        $debugStr .= "\$result = $result<br />";
        $totalUpdated++;
      }
    }
    if($totalUpdated > 0)
    {
      $debugStr .= "Graphical Diaplays updated to EEPROM<br />";
      $h2Header.="<font color=\"red\"><br />GLCD Names Updated</font>";
    }else{
      $debugStr .= "No GLCDs updated to EEPROM<br />";
      $h2Header.="<font color=\"red\"><br />No Names Updated</font>";
    }

  // update the LCD names
    $debugStr .= "Updating LCD Names<br />";
    $maxLCDcnt = $_POST["maxLCDcnt"];
    $debugStr .= "\$maxLCDcnt =  $maxLCDcnt<br />";
    for($updateCnt=0; $updateCnt<$_POST["maxLCDcnt"]; $updateCnt++)
    {
      $lcdAddress = $_POST["lcdAddress$updateCnt"];
      $lcdNameStr = $_POST["lcdName$updateCnt"];
      $query = "SELECT * from `lcdNames` where `address`='".$lcdAddress."' AND `netID`='".$netID."'";
      $debugStr .= $query."<br />";
      $result = mysqli_query($link,$query);


      if($result === FALSE)
      {
        $debugStr .="name query failed<br />";
      }else{
        $debugStr .="name success<br />";
      }

      $resultCnt = mysqli_num_rows($result);
      $debugStr .= "\$resultCnt = ".$resultCnt."<br />";
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
      $debugStr .= "\$query = $query <br />";
      $trimmedEscapedName = trim($escapedName);
      $debugStr .= "\$trimmedEscapedName = $trimmedEscapedName <br />";
      
      if($trimmedEscapedName !== "NULL")
      {
        $debugStr .= "\$query = $query<br />";

        $result = mysqli_query($link,$query);

        if($result === FALSE)
        {
          $debugStr .= "lcd1w name update failed<br />";
        }else{
          $debugStr .= "lcd1w name update success<br />";
        }
        $in = $updateLCD1wName." ".$lcdAddress." ".$lcdNameStr;
        $debugStr .= "\$in = $in<br />";
        $result = udpRequest($service_port, $port_address, $in);
        $debugStr .= "\$result = $result<br />";
        $totalUpdated++;
      }
    }

    if($totalUpdated > 0)
    {
      $in = $saveToEEPROM;
      $result = udpRequest($service_port, $port_address, $in);
      sleep(2);
      $in = $restoreStructures;
      $result = udpRequest($service_port, $port_address, $in);
      sleep(2);
      $debugStr .= "LCD displays updated to EEPROM<br />";
      $h2Header.="<font color=\"red\"><br />LCD Names Updated</font>";
    }else{
      $debugStr .= "No LCDs updated to EEPROM<br />";
      $h2Header.="<font color=\"red\"><br />No Names Updated</font>";
    }
  }

// Update Display  
  $debugStr .= "Checking for LCD 1-wire displays<br />";
  $in = $getLCDcnt."\n";
  $maxLCDcnt = udpRequest($service_port, $port_address, $in);
  $debugStr .= "maxLCDcnt = $maxLCDcnt<br />";

  if($maxLCDcnt > 0)
  {
    $formStr1="
      <table width=\"90%\" border=\"1\" cellspacing=\"0\" cellpadding=\"2\">
        <tr>
          <td align=\"center\" colspan=\"2\">
            <h2><font color=\"blue\"><strong>1-Wire&reg; 4x20 LCD Displays</strong></font></h2>
          </td>
        </tr>
        <tr>
          <td colspan=\"4\" align=\"center\">
            <input type=\"hidden\" name=\"maxLCDcnt\" value=\"".$maxLCDcnt."\">
          </td>
        </tr>";
   
    for($scCnt=0; $scCnt < $maxLCDcnt; $scCnt++)
    {
      $in = $getLCDstatus.$scCnt."\n";
      $out = udpRequest($service_port, $port_address, $in);
      $debugStr .="\$out = $out<br />";
      $lcdArray = explode(";", $out);
      $lcd1wAddress = $lcdArray[$lcdAddrIndex];
      $lcdType = $lcdArray[$lcdTypeIndex];
      $lcdName = $lcdArray[$lcdNameIndex];
      $lcd1wAddrArray = explode(",", $lcd1wAddress);
      $lcdID = $lcd1wAddrArray[$lcdIdIndex];

      $debugStr .= "lcd1wAddress = $lcd1wAddress<br />";
      $debugStr .= "lcdType = $lcdType<br />";
      $debugStr .= "lcdName = $lcdName<br />";
      $debugStr .= "lcdID = $lcdID<br />";

/*      
      if(trim($lcdName) === "__UNASSIGNED___")
      {
        $query = "SELECT `name` FROM `lcdNames` WHERE `address`='$lcd1wAddress' AND `netID`='$netID'";
        $debugStr .= "query = $query<br />";
        $result = mysqli_query($link, $query);
        $rowCnt = mysqli_num_rows($result);
        $debugStr .= "rowCnt = $rowCnt<br />";

        if(($result !== FALSE) && ($rowCnt > 0))
        {
            $debugStr .="lcd1w name query success, name = ";
          $resultObj = mysqli_fetch_object($result);
          $lcdName = $resultObj->name;
            $debugStr .=$resultObj->name."<br />";
        }else{
            $debugStr .="lcd1w name query failed<br />";
        }
      }
      mysqli_free_result($result);
*/
      $debugStr .="\$lcdID = $lcdID, \$dsLCD = $dsLCD<br />"; 

      if(trim($lcdID) === trim($dsLCD))
      {
        $formStr1.="
          <td align=\"center\">
            <input type=\"hidden\" name=\"lcdAddress".$scCnt."\" value=\"".$lcd1wAddress."\">$lcd1wAddress
          </td>
          <td align=\"center\">
            <input type=\"text\" size=\"25\" name=\"lcdName".$scCnt."\" value=\"".$lcdName."\">
          </td>
        </tr>
        ";
       }
    }
  }else{
    $formStr1.="
      <table width=\"90%\" border=\"1\" cellspacing=\"0\" cellpadding=\"2\">
        <tr>
          <td colspan=\"4\" align=\"center\">
            <font color=\"blue\"><h2>No 1-Wire&reg; 4x2 LCDs Available</h2></font>
          </td>
        </tr>
    ";
  }
  $formStr1.="
    </table>";

  $debugStr .= "Checking for GLCD 1-wire displays<br />";
  $in = "$getGLCDcnt\n";
  $out = udpRequest($service_port, $port_address, $in);
  $maxGLCDcnt = trim($out);
  $debugStr .= "\$maxGLCDcnt = $maxGLCDcnt<br />";

  if($maxGLCDcnt > 0)
  {
    $formStr2="
      <table width=\"90%\" border=\"1\" cellspacing=\"0\" cellpadding=\"2\">
        <tr>
          <td align=\"center\" colspan=\"2\">
            <h2><font color=\"blue\"><strong>1-Wire&reg; 7\" CTE GLCD Displays</strong></font></h2>
          </td>
        </tr>
        <tr>
          <td colspan=\"4\" align=\"center\">
            <input type=\"hidden\" name=\"maxGLCDcnt\" value=\"".$maxGLCDcnt."\">
          </td>
        </tr>";
  }else{
    $formStr2.="
      <table width=\"90%\" border=\"1\" cellspacing=\"0\" cellpadding=\"2\">
        <tr>
          <td colspan=\"4\" align=\"center\">
            <font color=\"blue\"><h2>No 1-Wire&reg; Graphical LCDs Available</h2></font>
          </td>
        </tr>";
  }

  for($scCnt=0; $scCnt < $maxGLCDcnt; $scCnt++)
  {
    $in = $getGLCDstatus.$scCnt."\n";
    $out = udpRequest($service_port, $port_address, $in);
    $debugStr .= "\$out = $out<br />";
    $glcdArray = explode(";", $out);
    $glcdAddressArray = explode(",", $glcdArray[$glcdAddrIndex]);
    $glcdAddrArrayStr = $glcdArray[$glcdAddrIndex];
    $debugStr .="\$glcdAddrArrayStr = $glcdAddrArrayStr<br />";
    $glcdID = $glcdAddressArray[$glcdIdIndex];
    $glcdName = $glcdArray[$glcdNameIndex];
    $debugStr .="\$glcdName = $glcdName<br />";

/*
    if(trim($glcdName) === "NULL")
    {
      $query = "SELECT `name` FROM `glcdNames` WHERE `address`='".trim($glcdAddrArrayStr)."' AND `netID`='$netID'";
      $debugStr .="\$query = $query<br />";
      $result = mysqli_query($link, $query);
      $rowCnt = mysqli_num_rows($result);
      $debugStr .="number of rows returned = $rowCnt<br />";
      if(($result !== FALSE) && ($rowCnt > 0))
      {
        $debugStr .="GLCD name query success, name = ";
        $resultObj = mysqli_fetch_object($result);
        $glcdName = $resultObj->name;
          echo $resultObj->name."<br />";
      }else{
        $debugStr .="GLCD name query failed<br />";
      }
    }
      
      mysqli_free_result($result);
*/    
    $debugStr .="\$glcdID = $glcdID, \$dsGLCDP = $dsGLCDP<br />";
    if( trim($glcdID) === trim($dsGLCDP) )
    {
      $debugStr .="Adding $glcdAddrArrayStr<br />";
      $formStr2.="
        <tr>
          <td align=\"center\">
            ".$glcdAddrArrayStr."
            <input type=\"hidden\" name=\"glcdAddress".$scCnt."\" value=\"".trim($glcdAddrArrayStr)."\">
          </td>
          <td align=\"center\">
            <input type=\"text\" size=\"25\" name=\"glcdName".$scCnt."\" value=\"".$glcdName."\">
          </td>
        </tr>";
    }
  }

  $formStr2.="
    </table>";  
    
  $bodyStr ="
    <body>
      $logoStr
      <!-- Table for Main Body -->
      <table width=\"100%\" border=\"1\" cellspacing=\"0\" cellpadding=\"2\">
        <tr>
          <td colspan=\"4\">
            $menuStr
            <br />
          </td>
        <tr>
          <td align=\"center\" colspan=\"4\">
            $h2Header
          </td>
        </tr>";

  $debugStr .= "
        </td>
      </tr>
    </table>";
  
  $formStr.="
    <table width=\"100%\" border=\"1\" cellspacing=\"0\" cellpadding=\"2\">
      <tr>
        <td width=\"50%\" align=\"center\">$formStr1</td><td width=\"50%\" align=\"center\"> $formStr2</td>
      </tr>
      <tr>
        <td colspan =\"4\" align=\"center\">
          <input type=\"submit\" value=\"UPDATE NAMES\">
          </form>
        </td>
      </tr>
    <table>
  ";
  $formStr .="
        <tr>
          <td colspan=\"4\">
            $menuStr
          </td>
        </tr>
      </table>";

  
  $h2Header.="</h2>";

  echo $headStr;
  echo $bodyStr;
  echo $formStr;
//  echo $debugStr;
  echo $footStr;
  $deBugStr = str_replace("<br />","\n",$debugStr);
//  fwrite($debugFile, $deBugStr."\n\n\n");
//  fclose($debugFile);
?>
  


