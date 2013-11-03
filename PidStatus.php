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
//    echo "PidStatus.php: \$netID = ".$netID.", \$service_port = ".$service_port.", \$port_address = ".$port_address.", \$netName = ".$netName."<br />";
    $headStr = "
      <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">
      <title> PID Status </title>
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
          $(\"#container\").load(\"updatePidStatusWithMySQL.php?netID=$netID&service_port=$service_port&port_address=$port_address&netName=$netName\");
         var refreshId = setInterval(function() {
            $(\"#container\").load(\"updatePidStatusWithMySQL.php?netID=$netID&service_port=$service_port&port_address=$port_address&netName=$netName\");
         }, 1000);
         $.ajaxSetup({ cache: false });
      });
      </script>";
  }else{
    $headStr = "
      <meta charset=\"UTF-8\">
      <meta http-equiv=\"refresh\" content=\"1;url=http://localhost/Test/index.php\">
      <script type=\"text/javascript\">
          window.location.href = \"http://localhost/Test/index.php\"
      </script>";
  }
  echo $headStr;
?>
</head>
  <body>
    <?php 

      if(isset($_POST["masterPidStop"]) && $_POST["masterPidStop"] === "masterPidStop")
      {
        $in = $masterPidStop."\n";
		    $chipX = udpRequest($service_port, $port_address, $in);
      }
      
      if(isset($_POST["pidEnable"]) && ($_POST["pidEnable"] === "pidEnable" ) && isset($_POST["pidCnt"]))
      {  
        $in = $setPidArray." ".$_POST["pidCnt"]." 1\n";
		    $eepromStatus = udpRequest($service_port, $port_address, $in);
      }
      
      if(isset($_POST["pidDisable"]) && ($_POST["pidDisable"] === "pidDisable" ) && isset($_POST["pidCnt"]))
      {  

        $in = $setPidArray." ".$_POST["pidCnt"]." 0\n";
		    $eepromStatus = udpRequest($service_port, $port_address, $in);
      }
    ?>
    
    <!-- Table for Main Body -->
    <table width="100%" border="0" cellspacing="0" cellpadding="0">
      <tr>
        <td>
        <?php 
            include ("menu.php");
            if(isset($_POST["pidSetup"]) && $_POST["pidSetup"] === "pidSetup")
            {
//              echo "<br />pidCnt=".$_POST["pidCnt"]."<br />pidEnable=".$_POST["pidEnable"]."<br />pidTempAddr=".$_POST["pidTempAddr"]."<br />pidSetPoint=".$_POST["pidSetPoint"]."<br />pidSwitchAddr=".$_POST["pidSwitchAddr"]."<br />pidKp=".$_POST["pidKp"]."<br />pidKi=".$_POST["pidKi"]."<br />pidKd=".$_POST["pidKd"]."<br />pidDirection=".$_POST["pidDirection"]."<br />pidWindowSize=".$_POST["pidWindowSize"];
              $pidCnt = $_POST["pidCnt"];
              $updatePidStr = "M ".$_POST["pidCnt"]." ".$_POST["pidEnable"]." ".$_POST["pidTempAddr"]." ".$_POST["pidSetPoint"]." ".$_POST["pidSwitchAddr"]." ".$_POST["pidKp"]." ".$_POST["pidKi"]." ".$_POST["pidKd"]." ".$_POST["pidDirection"]." ".$_POST["pidWindowSize"]."\n";
//              echo "<br />updatePidStr = ".$updatePidStr."<br />";
              $pidIn = $updatePidStr;
//              echo "pidIn = ".$pidIn.", length = ".strlen($pidIn)."<br />";
		          $pidOut = udpRequest($service_port, $port_address, $pidIn);
//              echo "pidOut = ".$pidOut."<br />";

		          $in = $saveToEEPROM."\n";
		          $out = udpRequest($service_port, $port_address, $in);
		          sleep(2);

              $query = "SELECT * FROM pid WHERE netID=$netID AND id=$pidCnt";
              $result=mysqli_query($link, $query);
              if(mysqli_num_rows($result) === 0)
              {
                $query = "INSERT INTO pid SET enabled='".$_POST["pidEnable"]."',tempAddr='".$_POST["pidTempAddr"]."',setpoint='".$_POST["pidSetPoint"]."',switchAddr='".$_POST["pidSwitchAddr"]."',kp='".$_POST["pidKp"]."',ki='".$_POST["pidKi"]."',kd='".$_POST["pidKd"]."',direction='".$_POST["pidDirection"]."',windowSize='".$_POST["pidWindowSize"]."', id='".$_POST["pidCnt"]."', netID='".$netID."'";
              }else{
                $query = "UPDATE pid SET enabled='".$_POST["pidEnable"]."',tempAddr='".$_POST["pidTempAddr"]."',setpoint='".$_POST["pidSetPoint"]."',switchAddr='".$_POST["pidSwitchAddr"]."',kp='".$_POST["pidKp"]."',ki='".$_POST["pidKi"]."',kd='".$_POST["pidKd"]."',direction='".$_POST["pidDirection"]."',windowSize='".$_POST["pidWindowSize"]."' WHERE id='".$_POST["pidCnt"]."' AND netID='".$netID."'";
              }
              mysqli_free_result($result);
//              echo $query."<br />";
              $result=mysqli_query($link, $query);
/*
              if($result === FALSE)
              {
                echo "query failed";
              }else{
                echo "query success";
              }
              echo "<br />result = ".$result."<br />";
*/
              mysqli_free_result($result);
            }
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
