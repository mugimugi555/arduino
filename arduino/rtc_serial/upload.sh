#!/bin/bash

# Arduino CLIのパス
ARDUINO_CLI="arduino-cli"

# ボード情報
BOARD="arduino:avr:uno"      # 使用するボード名
PORT="/dev/ttyUSB0"          # 使用するポート（適宜変更）

# スケッチのパス
SKETCH_PATH="rtc_serial.ino"  # スケッチのパス

# スケッチをアップロード
echo "スケッチをアップロード中..."
$ARDUINO_CLI upload -p $PORT --fqbn $BOARD $SKETCH_PATH

if [ $? -ne 0 ]; then
  echo "スケッチのアップロードに失敗しました。"
  exit 1
fi

echo "スケッチのアップロードが完了しました。"

# 少し待機（Arduinoの再起動を待つ）
sleep 2

# 現在の時刻を取得
CURRENT_TIME=$(date "+%Y-%m-%d %H:%M:%S")
echo "現在の時刻: $CURRENT_TIME"

# 時刻をArduinoに送信
echo "時刻を送信中..."
echo -e "$CURRENT_TIME\n" > $PORT

echo "時刻の送信が完了しました。"
