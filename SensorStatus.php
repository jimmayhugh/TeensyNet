<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<?php
  include_once("accessDatabase.php");
  include_once("udpRequest.php");
  if( (isset($_POST["netID"]) && $_POST["netID"] >= 0) || (isset($_GET["netID"]) && $_GET["netID"] >= 0) )
  {
    if( (isset($_POST["netID"]) && $_POST["netID"] >= 0) )
    {
      $netID        = $_POST["netID"];
      $service_port = $_POST["service_port"];
      $port_address = $_POST["port_address"];
      $netName      = $_POST["netName"];
    }else{
      $netID        = $_GET["netID"];
      $service_port = $_GET["service_port"];
      $port_address = $_GET["port_address"];
      $netName      = $_GET["netName"];
    }
//    echo "SensorStatus.php: \$netID = ".$netID.", \$service_port = ".$service_port.", \$port_address = ".$port_address.", \$netName = ".$netName."<br />";

    $headStr = "
      <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">
      <title> Sensor / Switch Update </title>
      <link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"/>
      <script type=\"text/javascript\" src=\"//ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js\"></script>
      <!-- <script type=\"text/javascript\" src=\"js/jquery.js\"></script> -->
      <script>
        $(document).ready(function() {
          $(\"#container\").load(\"updateStatus.php?netID=$netID&service_port=$service_port&port_address=$port_address&netName=$netName\");
       var refreshId = setInterval(function() {
          $(\"#container\").load(\"updateStatus.php?netID=$netID&service_port=$service_port&port_address=$port_address&netName=$netName\");
       }, 1000);
       $.ajaxSetup({ 
                      cache: false
                   });
      });
      </script>
      </script>
      <style>
        input[type='text'] { font-size: 18px; text-align: center;}
        input:focus, textarea:focus{background-color: lightgrey;}
        select[type='text'] { font-size: 18px; text-align: center;}
      </style>";
  }else{
    $headStr = "
      <meta charset=\"UTF-8\">
      <meta http-equiv=\"refresh\" content=\"1;url=index.php\">
      <script type=\"text/javascript\">
          window.location.href = \"index.php\"
      </script>";
  }
  echo $headStr;
?>
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
      include ("menu.php");
    ?>
    </td>
  </tr>
  <tr>
    <td  align="center" colspan="6">
      <table width="100%" border="1" cellspacing="0" cellpadding="2">
        <tr>
          <td align="center" border="2" colspan="6">
            <?php
              $titleStr = "<h2><font color = \"blue\">Sensor / Switch Status <br /> ".$netName."</font></h2>";
              echo $titleStr;
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
      include ("menu.php");
    ?>
    </td>
  </tr>
</table>
</body></html>
