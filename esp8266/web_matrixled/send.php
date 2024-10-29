<?php

// https://github.com/Xowap/PHP-Serial
// php send_serial.php "送信するメッセージ"

// php-serial.php を読み込む
require_once("php-serial/php_serial.class.php");

// 引数からメッセージを取得（引数がない場合はデフォルトメッセージ）
$message = isset($argv[1]) ? $argv[1] : "デフォルトのメッセージ";

// シリアルポートの設定
$serial = new phpSerial();
$serial->deviceSet("/dev/ttyUSB0"); // シリアルデバイスパス
$serial->confBaudRate(115200);
$serial->confParity("none");
$serial->confCharacterLength(8);
$serial->confStopBits(1);
$serial->confFlowControl("none");

// シリアルポートをオープン
$serial->deviceOpen();

// メッセージを送信
$serial->sendMessage($message . "\n");
echo "メッセージを送信しました: $message\n";

// シリアルポートをクローズ
$serial->deviceClose();
?>

