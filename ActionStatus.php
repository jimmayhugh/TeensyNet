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

  if(isset($_POST["masterStop"]) && $_POST["masterStop"] === "masterStop")
  {
    $in = $masterStop."\n";
    $result = udpRequest($service_port, $port_address, $in);
  }


//    echo "SensorStatus.php: \$netID = ".$netID.", \$service_port = ".$service_port.", \$port_address = ".$port_address.", \$netName = ".$netName."<br />";
    $headStr = "
      <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">
      <title> Action Status </title>
      <link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"/>
      <style type=\"text/css\" rel=\"stylesheet\">
        #masterStop {
                      background-color: #f00;
                      font-size: 100%;
                      width: 20em;
                      height: 5em;
                      font-weight: bold;
                    }
      </style>
      <script type=\"text/javascript\" src=\"//ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js\"></script>
      <!-- <script type=\"text/javascript\" src=\"js/jquery.js\"></script> -->
      <script>
       $(document).ready(function() {
          $(\"#container\").load(\"updateActionStatusWithMySQL.php?netID=$netID&service_port=$service_port&port_address=$port_address&netName=$netName\");
         var refreshId = setInterval(function() {
            $(\"#container\").load(\"updateActionStatusWithMySQL.php?netID=$netID&service_port=$service_port&port_address=$port_address&netName=$netName\");
         }, $udpUpdate);
         $.ajaxSetup({ cache: false });
      });
      </script>";
  }else{
    $headStr = "
      <meta charset=\"UTF-8\">
      <meta http-equiv=\"refresh\" content=\"1;url=http://localhost/Test/index.php\">
      <script type=\"text/javascript\">
          window.location.href = \"http://localhost/index.php\"
      </script>";
  }
  echo $headStr;
?>
</head>
  <body>
    <?php 
      include_once("header.html");
    ?> 
    <!-- Table for Main Body -->
    <table width="100%" border="0" cellspacing="0" cellpadding="0">
      <tr>
        <td>
        <?php 
            include ("menu.php");
          ?>
        </td>
      </tr>
      <tr>
        <td valign ="top"">
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
