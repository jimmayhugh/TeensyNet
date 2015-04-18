<?php
$headStr ="
<html>
 <!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">
 <head>";
  if( (isset($_POST["netID"]) && $_POST["netID"] >= 0) ||
      (isset($_GET["netID"]) && $_GET["netID"] >= 0) )
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
    $headStr .= "
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
    $headStr .= "
      <meta charset=\"UTF-8\">
      <meta http-equiv=\"refresh\" content=\"1;url=http://localhost/index.php\">
      <script type=\"text/javascript\">
          window.location.href = \"http://localhost/index.php\"
      </script>";
  }
    $headStr .= "
  </head>
  <body>";
?>
