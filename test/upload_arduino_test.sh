#!/bin/bash

# ファイルのパスを配列に格納
files=(
  "bmp180/bmp180.ino"
  "mq3_lcd/mq3_lcd.ino"
  "oled/oled.ino"
)

cd ../basic
# 各ファイルに対して処理を実行
for file in "${files[@]}"; do
  bash upload_arduino.sh "$file"
done
cd ../test
