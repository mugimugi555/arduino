#!/bin/bash

# bash send_serial.sh "送信するメッセージ"

# 引数からメッセージを取得（引数がない場合はデフォルトメッセージ）
message=${1:-"デフォルトのメッセージ"}

# シリアルポート（ESP8266のポートに変更してください）
serial_port="/dev/ttyUSB0"

# メッセージをシリアル送信
echo -ne "$message\r" > $serial_port
echo "メッセージを送信しました: $message"
