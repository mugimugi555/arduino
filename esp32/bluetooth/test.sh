#!/bin/bash

# ファイルのパスを配列に格納
files=(
  "bluetooth_auto_scroller/bluetooth_auto_scroller.ino"
  "bluetooth_mouse_keyboard/bluetooth_mouse_keyboard.ino"
)

# 各ファイルに対して処理を実行
for file in "${files[@]}"; do
  bash upload_esp32_bluetooth.sh "$file"
done
