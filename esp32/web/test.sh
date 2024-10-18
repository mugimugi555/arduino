#!/bin/bash

# WiFi SSID、パスワード、ホスト名を変数化
ssid="wifissid"
password="wifipasswd"
hostname="hostname"

# ファイルのパスを配列に格納
files=(
  "web_bme280/web_bme280.ino"
  "web_dht11/web_dht11.ino"
  "web_mq/web_mq.ino"
)

# 各ファイルに対して処理を実行
for file in "${files[@]}"; do
  bash upload_esp32_web.sh "$file" "$ssid" "$password" "$hostname"
done
