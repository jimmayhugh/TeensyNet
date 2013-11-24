<?php

  include_once("udpRequest.php");
  include_once("accessDatabase.php");
  
  $actionDisable = 0;
  $actionEnable  = 1;

  if(isset($_GET["netName"])) 
  {
    $service_port=$_GET["service_port"];
    $port_address=$_GET["port_address"];
    $netName=$_GET["netName"];
    $netID=$_GET["netID"];
//      echo "updateStatus.php: netID = $netID, service_port = $service_port, port_address = $port_address, netName = $netName <br />";
  }
  
  if(isset($_POST["netName"])) 
  {
    $service_port=$_POST["service_port"];
    $port_address=$_POST["port_address"];
    $netName=$_POST["netName"];
    $netID=$_POST["netID"];
//      echo "updateStatus.php: netID = $netID, service_port = $service_port, port_address = $port_address, netName = $netName <br />";
  }
  

  $in = $getMaxActions."\n";
  $chipX = udpRequest($service_port, $port_address, $in);
  $actionNum = trim($chipX);
  
  $in = $getActionStatus."\n";
  $chipX = udpRequest($service_port, $port_address, $in);
   
  $chipY = explode(";", $chipX);
  $chipYcount = count($chipY);

//  echo "masterStop =".$result."<br />";
//  echo "eepromStatus = ".$eepromStatus."<br />";
//  echo "getMaxActions = ".$actionNum."<br />";
//  echo "getActionStatus = ".$chipX."<br />";

/*
  echo "chipYCount = ".$chipYcount."<br />";
  for($chipArrayCnt = 0; $chipArrayCnt < $chipYcount; $chipArrayCnt++)
  {
    echo "chipArrayCnt = ".$chipArrayCnt.", chipY[".$chipArrayCnt."] = ".$chipY[$chipArrayCnt]."<br />";
  }
*/
  $bodyStr="<table width=\"100%\" border=\"2\" cellspacing=\"0\" cellpadding=\"5\">
              <tr>
                <td align=\"center\" colspan=\"5\">
                  <h2><font color =\"blue\">Action Status<br />".$netName."</font></h2>
                </td>
               <tr>
                <td align=\"center\" colspan=\"5\">
                  </form>
                  <font color=\"red\">
                    USE THIS BUTTON ONLY WHEN NECESSARY!!
                    <br />
                    </font>
                  <form method=\"post\" action=\"ActionStatus.php\">
                    <input type=\"hidden\" name=\"masterStop\" value=\"masterStop\">
                    <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
                    <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
                    <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">
                    <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
                    <input type=\"submit\" id=\"masterStop\" value=\"Stop All Actions\">
                  </form>
                  <font color=\"red\">
                    USE THIS BUTTON ONLY WHEN NECESSARY!!
                    <br />
                    </font>
                 </td>
               </tr>
               <tr>";
  
  for($x=0;$x<$actionNum;$x++)
  {
    
    $chipXArray = explode(",", $chipY[$x]);
    
    $trimStr = trim($chipXArray[0]);
    $trimStr1 = trim($chipXArray[1]);
    $trimStr2 = trim($chipXArray[2]);
    $trimStr3 = trim($chipXArray[3]);
    $trimStr4 = trim($chipXArray[4]);
    $trimStr5 = trim($chipXArray[5]);
    
    $query = "SELECT * FROM action WHERE id=".$x." AND netID=".$netID;
    $result = mysqli_query($link,$query);
    if($result != NULL && mysqli_num_rows($result) > 0)
    {
      $finfo = mysqli_fetch_object($result);
      $temperatureAddress = $finfo->tempAddr;
      $tooHotAddress = $finfo->thAddr;
      $tooColdAddress = $finfo->tcAddr;
//      $bodyStr .= "<br />action = ".$x."&nbsp;&nbsp;&nbsp;".$temperatureAddress."&nbsp;&nbsp;&nbsp;".$tooHotAddress."&nbsp;&nbsp;&nbsp;".$tooColdAddress."<br />";
      mysqli_free_result($result);
    }
    
    $query = "select * from chipNames where address='".$temperatureAddress."'";
    $result = mysqli_query($link,$query);
    if($result != NULL && mysqli_num_rows($result) > 0)
    {
      $finfo = mysqli_fetch_object($result);
      $temperatureName = $finfo->name;
//      $bodyStr .= "\$temperatureName = $temperatureName <br />";
      mysqli_free_result($result);
    }
//    if($temperatureName == ""){$temperatureName = "Temp Sensor ".$x;}
    
    $query = "select * from chipNames where address='".$tooHotAddress."'";
    $result = mysqli_query($link,$query);
    if($result != NULL && mysqli_num_rows($result) > 0)
    {
      $finfo = mysqli_fetch_object($result);
      $thName = $finfo->name;
      mysqli_free_result($result);
    }
    if($thName == ""){$thName = "Hot Switch ".$x;}

    $query = "select * from chipNames where address='".$tooColdAddress."'";
    $result = mysqli_query($link,$query);
    if($result != NULL && mysqli_num_rows($result) > 0)
    {
      $finfo = mysqli_fetch_object($result);
      $tcName = $finfo->name;
      mysqli_free_result($result);
    }
    if($tcName == ""){$tcName = "Cold Switch ".$x;}

    $bodyStr.= 
    "<div id=\"action".$x."\">
      <td valign=\"top\" align=\"center\">
        <table border=\"1\" width=\"25%\" cellspacing=\"0\" cellpadding=\"10\">
          <tr>
            <td align=\"center\" colspan=\"4\">";
    if($trimStr === "0")
    {
      $bodyStr .= 
        "<font color=\"red\"><strong>DISABLED</strong></font><br />";
    }else if($trimStr === "1"){
      $bodyStr .= 
        "<font color=\"green\"><strong>ENABLED</strong></font><br />";
    }else{
      $bodyStr .=
        "<font color=\"yellow\"><strong>UNKNOWN = ".$trimStr."</strong></font><br />";
    }
    
    $bodyStr .= 
        "<div id=\"enaction".$x." \"style=\"text-align:center; vertical-align:middle; min-width:75px\">
        <script>
          jQuery(document).ready(function(){
            jQuery('#ajaxAction".$x."disable').click(function(event){
             jQuery.ajax({
               cache: false,
               type: \"GET\",
               url: \"setActionEnable.php\",
               data: \"action=".$x."&state=".$actionDisable."&service_port=$service_port&port_address=$port_address&netName=$netName\"
              });
            });
          });

        jQuery(document).ready(function(){
          jQuery('#ajaxAction".$x."enable').click(function(event){
            jQuery.ajax({
               cache: false,
              type: \"GET\",
              url: \"setActionEnable.php\",
              data: \"action=".$x."&state=".$actionEnable."&service_port=$service_port&port_address=$port_address&netName=$netName\"
              });
          });
        });
      </script>
      <style>
        #ajax{cursor:pointer;}
        #ajaxAction".$x."enable{margin-bottom:0px;}
        #ajaxAction".$x."enable span{display:block;max-width:75px;padding:1px;background-color:#00FF00;color:#000;text-align:center;border-width:1; border-color:gray;}
        #ajaxAction".$x."enable span:hover{display:block;max-width:75px;padding:1px;background-color:#000;color:#FFFF00;;text-align:center;border-width:1; border-color:gray;}
        #ajaxAction".$x."disable{margin-bottom:0px;;text-align:center;border-width:1; border-color:gray;}
        #ajaxAction".$x."disable span{display:block;max-width:75px;padding:1px;margin-bottom:0px;background-color:#FF0000;color:#000;border-width:1;text-align:center; border-color:gray;}
        #ajaxAction".$x."disable span:hover{display:block;max-width:75px;padding:1px;background-color:#000;color:#FFFF00;;text-align:center;border-width:1; border-color:gray;}
      </style>";
      
     $bodyStr .=
        "</div>";

      if($trimStr1 !== "NULL")
      {
       $bodyStr .=
        "<table border=\"0\" width=\"25%\" cellspacing=\"0\" cellpadding=\"0\">
          <tr>
            <td>
              &nbsp;
            </td>
            <td style=\"border-style:solid;border-width:1; border-color:gray;\">";
        if($trimStr === "0")
        {
          $bodyStr .=    
            "<div id=\"ajaxAction".$x."enable\" style=\"text-align:center; vertical-align:middle; min-width:75px; border-color:gray;\">
              <span><strong>ENABLE</strong></span>
            </div>";
        }else{
          $bodyStr .=    
            "<div id=\"ajaxAction".$x."disable\" style=\"text-align:center; vertical-align:middle; min-width:75px border-color:gray;\">
              <span><strong>DISABLE</strong></span>
            </div>";
        }
       $bodyStr .=
        "</td><td>&nbsp;</td></tr></table>";
      }
    $bodyStr .= 
        "<form method=\"post\" action=\"ActionDataWithMySQL.php\">
           <input type=\"hidden\" name=\"actionCnt\" value=\"".$x."\">
           <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
           <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
           <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">
           <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
           <input type=\"submit\" value=\"MODIFY\">
         </form>";

    $bodyStr .=
       "<form method=\"post\" action=\"plotData.php\">
         <input type=\"hidden\" name=\"graphId\" value=\"".$x."\">
         <input type=\"hidden\" name=\"netID\" value=\"".$netID."\">
         <input type=\"hidden\" name=\"actionCnt\" value=\"".$x."\">
         <input type=\"hidden\" name=\"service_port\" value=\"".$service_port."\">
         <input type=\"hidden\" name=\"port_address\" value=\"".$port_address."\">
         <input type=\"hidden\" name=\"netName\" value=\"".$netName."\">";

    if($trimStr === "1")
    {
      $bodyStr .=
         "<input type=\"submit\" value=\"GRAPH\">";
    }else{
      $bodyStr .=
         "<input type=\"submit\" value=\"GRAPH\" disabled>";
    }
      $bodyStr .= "</form>";

    $bodyStr .= 
       "</td>
     </tr>
     <tr>
      <td align=\"center\" colspan=\"4\">";
    if(is_numeric($trimStr1) && ($trimStr1 >= -76 && $trimStr1 < 215))
    {
      if($trimStr1 <= $trimStr4)
      {
        $bodyStr .=
          "<div style=\"vertical-align:middle; min-height:50px;\">".$temperatureName."<br />
           <font size=\"10\" color=\"blue\"><strong>".$trimStr1."&deg;</strong></font>
           </div>";
      }else if($trimStr1 >= $trimStr5){
        $bodyStr .=
          "<div style=\"vertical-align:middle; min-height:50px;\">".$temperatureName."<br />
           <font size=\"10\" color=\"red\"><strong>".$trimStr1."&deg;</strong></font>
           </div>";
      }else{
        $bodyStr .=
          "<div style=\"vertical-align:middle; min-height:50px;\">".$temperatureName."<br />
           <font size=\"10\" color=\"green\"><strong>".$trimStr1."&deg;</strong></font>
           </div>";
      }
    }else{
      $bodyStr .=
        "<div style=\"vertical-align:middle; min-height:50px;\">
          <font color=\"red\" size=\"5\"><strong>UNUSED</strong></font>
         </div>";
    }
    $bodyStr .= 
      "</td>
      </tr>
      <tr>
        <td align=\"center\" style=\"text-align:center; vertical-align:middle; min-width:150px\">
          <div id=\"tcswitch".$x."\">
            <script>
              jQuery(document).ready(function(){
                jQuery('#ajaxtc".$x."off').click(function(event){
                  jQuery.ajax({
                    cache: false,
                    type: \"GET\",
                    url: \"setActionSwitch.php\",
                    data: \"data=".$x.$tooCold."F&service_port=$service_port&port_address=$port_address&netName=$netName\"
                    });
                });
              });
              
              jQuery(document).ready(function(){
                jQuery('#ajaxtc".$x."on').click(function(event){
                  jQuery.ajax({
                    cache: false,
                    type: \"GET\",
                    url: \"setActionSwitch.php\",
                    data: \"data=".$x.$tooCold."N&service_port=$service_port&port_address=$port_address&netName=$netName\"
                    });
                });
              });
              
          </script>
          <style>
          #ajax{cursor:pointer;}
          #ajaxtc".$x."on{margin-bottom:20px;}
          #ajaxtc".$x."on span{display:block;min-width:75px;padding:3px;background-color:#00FF00;color:#000;}
          #ajaxtc".$x."on span:hover{display:block;min-width:75px;padding:3px;background-color:#000;color:#FFFF00;}
          #ajaxtc".$x."off{margin-bottom:20px;}
          #ajaxtc".$x."off span{display:block;min-width:75px;padding:3px;margin-bottom:20px;background-color:#FF0000;color:#000;}
          #ajaxtc".$x."off span:hover{display:block;min-width:75px;padding:3px;background-color:#000;color:#FFFF00;}
          </style>";
    if($trimStr2 === "N")
    {
        $bodyStr .= "
          Too Cold<br />".$trimStr4."&deg;<br \>".$tcName."<br />
          <font size=\"10\" color=\"green\"><strong>ON</strong></font>";
      if($trimStr === "0")
      {
        $bodyStr .=
          "<br />
          <div id=\"ajaxtc".$x."on\" style=\"text-align:center; vertical-align:middle; min-width:150px\">
            <span>ON</span>
          </div>
          <div id=\"ajaxtc".$x."off\" style=\"text-align:center; vertical-align:middle; min-width:150px\">
            <span>OFF</span>
          </div>";
       }
       $bodyStr .=
          "</td>
          </div>";
    }else if($trimStr2 === "F"){
        $bodyStr .= "
          Too Cold<br />".$trimStr4."&deg;<br \>".$tcName."<br />
          <font size=\"10\" color=\"red\"><strong>OFF</strong></font>";
      if($trimStr === "0")
      {
        $bodyStr .=    
          "<br />
          <div id=\"ajaxtc".$x."on\" style=\"text-align:center; vertical-align:middle; min-width:150px\">
            <span>ON</span>
          </div>
          <div id=\"ajaxtc".$x."off\" style=\"text-align:center; vertical-align:middle; min-width:150px\">
            <span>OFF</span>
          </div>";
      }
      $bodyStr .=
        "</div>";
    }else{
        $bodyStr .= "
            <div>
              <font color=\"red\"><strong>UNASSIGNED</stront</font>
            </div>";
    }
      $bodyStr .= 
        "</td>
        <td align=\"center\" style=\"text-align:center; vertical-align:middle; min-width:150px\">
        <div id=\"thswitch".$x."\">
          <script>
            jQuery(document).ready(function(){
              jQuery('#ajaxth".$x."off').click(function(event){
               jQuery.ajax({
                 cache: false,
                 type: \"GET\",
                 url: \"setActionSwitch.php\",
                 data: \"data=".$x.$tooHot."F&service_port=$service_port&port_address=$port_address&netName=$netName\"
                });
              });
            });

          jQuery(document).ready(function(){
            jQuery('#ajaxth".$x."on').click(function(event){
              jQuery.ajax({
                 cache: false,
                type: \"GET\",
                url: \"setActionSwitch.php\",
                data: \"data=".$x.$tooHot."N&service_port=$service_port&port_address=$port_address&netName=$netName\"
                });
            });
          });
        </script>
        <style>
          #ajax{cursor:pointer;}
          #ajaxth".$x."on{margin-bottom:20px;}
          #ajaxth".$x."on span{display:block;min-width:75px;padding:3px;background-color:#00FF00;color:#000;}
          #ajaxth".$x."on span:hover{display:block;min-width:75px;padding:3px;background-color:#000;color:#FFFF00;}
          #ajaxth".$x."off{margin-bottom:20px;}
          #ajaxth".$x."off span{display:block;min-width:75px;padding:3px;margin-bottom:20px;background-color:#FF0000;color:#000;}
          #ajaxth".$x."off span:hover{display:block;min-width:75px;padding:3px;background-color:#000;color:#FFFF00;}
        </style>";
    if($trimStr3 === "N")
    {
      $bodyStr .= "
          Too Hot<br />".$trimStr5."&deg;<br \>".$thName."<br />
          <font size=\"10\" color=\"green\"><strong>ON</strong></font>";
      if($trimStr === "0")
      {
        $bodyStr .=    
          "<br />
          <div id=\"ajaxth".$x."on\" style=\"text-align:center; vertical-align:middle; min-width:150px\">
            <span>ON</span>
          </div>
          <div id=\"ajaxth".$x."off\" style=\"text-align:center; vertical-align:middle; min-width:150px\">
            <span>OFF</span>
          </div>";
      }
      $bodyStr .=
        "</td>
      </div>";
    }else if($trimStr3 === "F"){
      $bodyStr .= "
        Too Hot<br />".$trimStr5."&deg;<br \>".$thName."<br />
        <font size=\"10\" color=\"red\"><strong>OFF</strong></font>";
      if($trimStr === "0")
      {
        $bodyStr .=
          "<br />
          <div id=\"ajaxth".$x."on\" style=\"text-align:center; vertical-align:middle; min-width:150px\">
            <span>ON</span>
          </div>
          <div id=\"ajaxth".$x."off\" style=\"text-align:center; vertical-align:middle; min-width:150px\">
            <span>OFF</span>
          </div>";
      }
      $bodyStr .= 
        "</td>
      </div>";
    }else{
        $bodyStr .= "
          <div>
            <font color=\"red\"><strong>UNASSIGNED</stront</font>
          </div>";
    }
      $bodyStr .= "</td></tr>\n";
      $bodyStr .= "</table></td></div>\n";
    if($x === 3 || $x === 5 || $x === 8){  $bodyStr .= "</tr><tr>\n";}
  }
  $bodyStr .= "</tr>\n</table>";

  echo $bodyStr;
  mysqli_close($link);

?>
