<?php

  include_once("udpRequest.php");
  include_once("accessDatabase.php");

  $tempStr           = "";
  $switchStr         = "";
  $unusedStr         = "";
  $switchjStr        = "";
  $maxTempPerLine    = 4;
  $chipName          = 3;
  $chipIDLoc         = 0;
  $chipInfoType      = 0;
  $chipInfoArrayData = 1;
  $chipInfoArrayName = 2;
  

  if(isset($_GET["service_port"]) && isset($_GET["port_address"]) && isset($_GET["netName"]) && isset($_GET["netID"]) ) 
  {
    $service_port = $_GET["service_port"];
    $port_address = $_GET["port_address"];
    $netName      = $_GET["netName"];
    $netID        = $_GET["netID"];
//      echo "updateStatus.php: netID = $netID, service_port = $service_port, port_address = $port_address, netName = $netName <br />";
  }
  
  if(isset($_POST["service_port"]) && isset($_POST["port_address"]) && isset($_POST["netName"]) && isset($_POST["netID"]) ) 
  {
    $service_port = $_POST["service_port"];
    $port_address = $_POST["port_address"];
    $netName      = $_POST["netName"];
    $netID        = $_POST["netID"];
//      echo "updateStatus.php: netID = $netID, service_port = $service_port, port_address = $port_address, netName = $netName <br />";
  }

  if(isset($_POST["modifyName"]))
  {
  }
  
  if(isset($_POST["updateName"]))
  {
    $service_port = $_POST["service_port"];
    $port_address = $_POST["port_address"];
    $netName      = $_POST["netName"];
    $netID        = $_POST["netID"];
    $chipName     = $_POST["chipName"];
    $chipAddress  = $_POST["chipAddress"];
    
    $chipName = str_replace(" ", "_", $chipName);
    $chipName = str_replace(",", "_", $chipName);
    $chipName = str_replace(";", "_", $chipName);

    $query = "SELECT name From chipNames WHERE address=$chipAddress AND netID=$netID";
    echo "\$query = $query <br />";
    $result = mysqli_query($link,$query);

    if($result === FALSE)
    {
      $query = "INSERT into chipNames SET 'name'='$chipName','address'='$chipAddress','netID'='$netID'";
      echo "\$query = $query <br />";
      mysqli_free_result($result);
      $result = mysqli_query($link,$query);
    }else{
      echo "query success<br />";
      $resultObj = mysqli_fetch_object($result);
      echo "MySQL chipName is ".$resultObj->name."<br />";
      if($chipName !== $resultObj->name)
      {
        $query = "UPDATE chipNames SET 'name'='$chipName' where 'address'='$chipAddress' AND 'netID'='$netID'";
        echo "\$query = $query <br />";
      }
      mysqli_free_result($result);
      $result = mysqli_query($link,$query);
    }
  }
    
  $in = $getChipCount."\n";
  $chipX = udpRequest($service_port, $port_address, $in);
//  echo $chipX." 1-wire devices found<br />";
//  echo "getAllStatus = ".$chipX."<br />";

  $tempCnt   = 0;
  $switchCnt = 0;  
  for($x=0,$y=0,$z=0;$x<$chipX;$x++)
  {
    $in = $showChip.$x;
    $chipInfo = udpRequest($service_port, $port_address, $in);
    $chipError = trim($chipInfo);
    if($chipError === "ERROR")
    {
      $x--;
      continue;
    }
    $chipInfoArray = explode(" ", $chipInfo);
    $chipName = $chipInfoArray[$chipInfoArrayName];
    $chipAddress = $chipInfoArray[$chipIDLoc];
    $chipType = explode(",", $chipInfoArray[$chipIDLoc]);
    $chipID = $chipType[$chipIDLoc];
/*
    echo "chipInfo = ".$chipInfo."<br />";
    echo "chipAddress = $chipAddress<br />";
    echo "chipID = ".$chipID."<br />";
    echo "chipData = ".$chipInfoArray[$chipInfoArrayData]."<br />";
    echo "chipName = ".$chipInfoArray[$chipInfoArrayName]."<br />";
*/
    switch($chipID)
    {
      case 0xAA:
      case 0x30:
      case 0x28:
      {
//        echo "temp device - ";
//        echo $chipInfoArray[$chipInfoArrayData]."&deg<br />";
        $tempStr .= "<div id=\"temp\"".$tempCnt."; position: relative; width: 50%>";
        if($chipName === "_____UNASSIGNED_____")
        {
          $tempStr .=
          "<td align=\"center\">
            Temp ".$tempCnt."
            <br /><br />";
        }else{
          $tempStr .=
          "<td align=\"center\">"
              .$chipName.
            "<br /><br />";
        }
        $tempCnt++;
        $tempStr .=
        "
          <font size=\"10\">
            <strong>
              ".$chipInfoArray[$chipInfoArrayData]."&deg;
            </strong>
          </font>
          <br /><br />
         </td>
         </div>";
         $y++;
         if($y == $maxTempPerLine)
         {
           $tempStr .= "
            </tr><tr>";
           $y = 0;
         }
        break;
      }
      case 0x12:
      {
//        echo "switch device - ";
        $trimStr = trim($chipInfoArray[$chipInfoArrayData]);
//        echo "data = ".$trimStr."<br />";
        $switchStr .= 
        "<div id=\"switch".$x."\"; position: relative; width: 25%;>
          <script>
            jQuery(document).ready(function(){
              jQuery('#ajax".$x."off').click(function(event){
                jQuery.ajax({
                  cache: false,
                  type: \"GET\",
                  url: \"setSwitch.php\",
                  data: \"switch=".$x."F&service_port=$service_port&port_address=$port_address&netName=$netName\"
                });
              });
            });

            jQuery(document).ready(function(){
              jQuery('#ajax".$x."on').click(function(event){
                jQuery.ajax({
                  cache: false,
                  type: \"GET\",
                  url: \"setSwitch.php\",
                  data: \"switch=".$x."N&service_port=$service_port&port_address=$port_address&netName=$netName\"
                });
              });
            });
          </script>
          <style>
            #ajax{cursor:pointer;}
            #ajax".$x."on{margin-bottom:20px;}
            #ajax".$x."on span{display:block;max-width:150px;padding:3px;background-color:#00FF00;color:#000;}
            #ajax".$x."on span:hover{padding:3px;background-color:#000;color:#FFFF00;}
            #ajax".$x."off{margin-bottom:20px;}
            #ajax".$x."off span{display:block;max-width:150px;padding:3px;margin-bottom:20px;background-color:#FF0000;color:#000;}
            #ajax".$x."off span:hover{padding:3px;background-color:#000;color:#FFFF00;}
          </style>";
          if($trimStr === "N"){
            if($chipName === "_____UNASSIGNED_____")
            {
              $switchStr .=
              "<td align=\"center\">Switch $switchCnt";
             }else{
               $switchStr .= 
               "<td align=\"center\">$chipName";
             }
            $switchStr .=
            "<br /><br />
            <font size=\"10\" color=\"green\">
                <strong>
                  ON
                </strong>
              </font>";
          }else if($trimStr === "F"){
            if($chipName === "_____UNASSIGNED_____")
            {
              $switchStr .=
              "<td align=\"center\">Switch $switchCnt";
             }else{
               $switchStr .= 
               "<td align=\"center\">$chipName";
             }
            $switchStr .=
              "<br /><br />
              <font size=\"10\" color=\"red\">
                <strong>
                  OFF
                </strong>
              </font>";
          }
          $switchCnt++;
          $switchStr .= 
            "<br /><br />
            <div id=\"ajax".$x."on\"><span>ON</span></div>
            <div id=\"ajax".$x."off\"><span>OFF</span></div>
            </td>";
          $switchStr .=
            "</div>"
            ;
          $z++;
          if($z === $maxTempPerLine)
          {
            $switchStr .= "
              </tr><tr>";
            $z = 0;
          }
        break;
      }
      default:
      {
//        echo "Fucking Mystery - $chipInfo<br />";
        break;
      }
    }
//    usleep(7500000);
  }

  echo "<table width=\"100%\" align=\"center\" border=\"2\">\n<tr>\n$tempStr</tr>\n</table>\n\n<table width=\"100%\" align=\"center\" border=\"2\">\n<tr>\n$switchStr</tr>\n</table>\n\n<!-- <table width=\"100%\" align=\"center\" border=\"2\">\n<tr>\n$unusedStr</tr>\n</table> -->";
?>
