<?php
$menuStr = 
"[<a href=\"index.php\">Home</a>]&nbsp;&nbsp;

[<a href=\"SensorStatus.php?stopUpdate=0&service_port=".$service_port."&port_address=".$port_address."&netID=".$netID."&netName=".$netName."\">Sensor Status</a>]&nbsp;&nbsp;

[<a href=\"ActionStatus.php?service_port=".$service_port."&port_address=".$port_address."&netID=".$netID."&netName=".$netName."\">Action Status</a>]&nbsp;&nbsp;

[<a href=\"PidStatus.php?service_port=".$service_port."&port_address=".$port_address."&netID=".$netID."&netName=".$netName."\">PID Status</a>]&nbsp;&nbsp;

[<a href=\"CheckForNewRestore.php?service_port=".$service_port."&port_address=".$port_address."&netID=".$netID."&netName=".$netName."\">Check For New / Restore</a>]&nbsp;&nbsp;

[<a href=\"UpdateNames.php?stopUpdate=1&service_port=".$service_port."&port_address=".$port_address."&netID=".$netID."&netName=".$netName."\">Update Chip Names</a>]&nbsp;&nbsp;

[<a href=\"updateGLCDnames.php?stopUpdate=1&service_port=".$service_port."&port_address=".$port_address."&netID=".$netID."&netName=".$netName."\">Update GLCD Names</a>]";

echo $menuStr;
?>

