<?php

// インストール方法
// composer require textalk/websocket
// composer require ratchet/pawl

// WebSocketクライアントライブラリを読み込み
require 'vendor/autoload.php'; // Ratchetなどのインストールが必要です

use Ratchet\Client\WebSocket;
use Ratchet\Client\Connector;
use React\EventLoop\Factory;

// ESP8266のIPアドレス
$ip_address = '192.168.1.xxx';

$loop = Factory::create();
$connector = new Connector($loop);

$wsUrl = "ws://{$ip_address}/ws"; // ESP8266のWebSocket URL

$connector($wsUrl)->then(function (WebSocket $conn) {

  echo "WebSocket接続が確立されました。\n";

  // メッセージ受信イベント
  $conn->on('message', function ($msg) use ($conn) {

    echo "受信メッセージ: $msg\n";

    // 必要に応じてデータを処理
    $data = json_decode($msg, true);
    var_dump( $data);

  });

  // エラー処理
  $conn->on('error', function ($e) {
    echo "エラー: {$e->getMessage()}\n";
  });

  // WebSocketを閉じる
  $conn->on('close', function () {
    echo "WebSocket接続が閉じられました。\n";
  });

}, function ($e) {
  echo "接続エラー: {$e->getMessage()}\n";
});

$loop->run();
