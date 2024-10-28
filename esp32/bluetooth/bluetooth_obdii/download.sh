#!/bin/bash

# シリアルポートを指定（例: /dev/ttyUSB0）
SERIAL_PORT="/dev/ttyUSB0"
BAUD_RATE="115200"

# コマンドをESP32に送信
echo "SEND_FILE" > "$SERIAL_PORT"

# 受信データをファイルに保存
cat "$SERIAL_PORT" > odb_data.csv
