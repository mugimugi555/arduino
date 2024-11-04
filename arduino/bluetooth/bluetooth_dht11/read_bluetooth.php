<?php

// sudo rfcomm bind /dev/rfcomm0 <JDY-32のアドレス>
// bluetoothctl
// power on
// agent on
// scan on
// pair <JDY-32のアドレス>
// 1234
// connect <JDY-32のアドレス>

$device = '/dev/rfcomm0';

$serial = fopen($device, "r+");

if (!$serial) {
  die("Bluetoothデバイスを開くことができませんでした");
}

while (true) {

  $data = trim( fgets($serial) );

  if ($data) {

    var_dump( $data );

  }
}

fclose($serial);
?>
