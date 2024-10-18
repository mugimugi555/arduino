#!/bin/bash

# ファイルのパスを配列に格納
files=(
  "upload_arduino"
  "upload_esp01_web"
  "upload_esp32_bluetooth"
  "upload_esp32_web"
  "upload_esp8266_web"
)

# 各ファイルに対して処理を実行
for file in "${files[@]}"; do
  bash "$file"_test.sh
done

