<?php

$host = '192.168.1.xxx'; // ESP8266のIPアドレス
$port = 12345; // ESP8266のリスニングポート番号

// ソケットを作成
$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
if ($socket === false) {
  die("ソケットの作成に失敗: " . socket_strerror(socket_last_error()));
}

// ESP8266に接続
$result = socket_connect($socket, $host, $port);
if ($result === false) {
  die("接続に失敗: " . socket_strerror(socket_last_error($socket)));
}

// データを受信
while (true) {
  $buffer = '';
  socket_recv($socket, $buffer, 2048, 0); // 受信バッファサイズ
  if( trim($buffer) != "" ){
    echo "受信データ: $buffer\n";
  }
}

// ソケットを閉じる
socket_close($socket);
