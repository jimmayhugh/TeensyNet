<?php
$username="teensynet";
$password="teensynet";
$database="teensynet";

$link = mysqli_connect("localhost",$username,$password);
@mysqli_select_db($link,$database) or die( "Unable to select database");

?>
