<?php
$device = '/dev/rfcomm0';

$serial = fopen($device, "r+");

if (!$serial) {
  die("Bluetoothデバイスを開くことができませんでした");
}

echo "温度、湿度、不快指数データを待機しています...\n";

while (true) {

  $data = fgets($serial);

  if ($data) {
    $json = json_decode($data, true);

    if ($json && isset($json['temperature']) && isset($json['humidity']) && isset($json['discomfortIndex'])) {
      echo "温度: " . $json['temperature'] . "℃\n";
      echo "湿度: " . $json['humidity'] . "%\n";
      echo "不快指数: " . $json['discomfortIndex'] . "\n";
    } else {
      echo "不正なデータ: $data\n";
    }
  }
}

fclose($serial);
?>
