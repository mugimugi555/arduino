#!/bin/bash

# シリアルポートのデバイス名（例：/dev/ttyUSB0）
SERIAL_PORT="/dev/ttyUSB0"
# ボーレート（例：115200）
BAUD_RATE=115200

# シリアルポートの設定
stty -F $SERIAL_PORT $BAUD_RATE cs8 -cstopb -parenb

# 送信するデータ
DATA="/item?minTemp=20&maxTemp=30"

# データを送信
echo $DATA > $SERIAL_PORT
