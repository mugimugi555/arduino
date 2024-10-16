<?php
require 'vendor/autoload.php';

use Lp\SerialPort\SerialPort;

// シリアルポートを設定します (デバイスパスは環境に応じて変更)
$serialPort = new SerialPort("/dev/ttyUSB0");

// シリアル通信の設定
$serialPort->configure(115200);

// ポートを開く
$serialPort->open();

// 無限ループでデータを読み続ける
while (true) {
    // データを読み取る
    $data = $serialPort->read(100);  // 100バイト読み込む

    // 取得したデータを表示
    if ($data) {
        echo "Received data: " . $data . "\n";
    }

    // CPUの使用率を下げるために少し待つ
    usleep(100000);  // 100ミリ秒待機
}

// ポートを閉じる (実際にはCtrl+Cで停止するまで到達しない)
$serialPort->close();
?>
