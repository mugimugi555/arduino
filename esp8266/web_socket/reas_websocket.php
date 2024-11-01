<?php

// composer require textalk/websocket

require 'vendor/autoload.php';

use WebSocket\Client;

// ESP8266のWebSocketサーバーのURLを指定
$client = new Client("ws://esp8266.local:port"); // portは適切に設定

while (true) {
  $message = $client->receive(); // メッセージを受信
  echo "Received: $message\n"; // メッセージを表示
}
?>
