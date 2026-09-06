<?php
// Define variables
date_default_timezone_set("America/Toronto");
$file = 'catwaterer.txt';

// If data is available
//if($json = file_get_contents("php://input")) {
if($json = json_decode(file_get_contents("php://input"), true)) {
  
  //$line = date('Y-m-d H:i:s')."\t".$json['data']."\t\t".$json['published_at']."\n";
  $line = date('Y-m-d H:i:s', strtotime($json['published_at']))."\t".$json['data']."\n";
  
  file_put_contents($file, $line, FILE_APPEND | LOCK_EX);
  
} else {
  echo "No data available!";
}
