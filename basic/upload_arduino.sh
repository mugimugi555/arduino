#!/bin/bash

# 引数の確認
if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <sketch_path>"
    exit 1
fi

# 引数
SKETCH_PATH=$1      # Arduinoスケッチファイルのパス

# Arduinoボード名とシリアル通信の設定（ATmega328Pの例）
BOARD_NAME=arduino:avr:uno  # Arduino UNOを使用する場合
SERIAL_BAND=9600            # 一般的なArduinoの通信速度

# ランダムなフォルダを作成
RANDOM_NAME=$(mktemp -u XXXXX)  # ランダムな名前を生成
RANDOM_DIR="/tmp/$RANDOM_NAME"
mkdir "$RANDOM_DIR"              # フォルダを作成
TEMP_SKETCH="$RANDOM_DIR/$RANDOM_NAME.ino"  # 同じ名前のファイルを作成
echo "Temporary sketch directory: $RANDOM_DIR"

# WIFISSID、WIFIPASSWD、HOSTNAMEの置き換え
sed -e "s/WIFISSID/$WIFI_SSID/" \
    -e "s/WIFIPASSWD/$WIFI_PASSWORD/" \
    -e "s/HOSTNAME/$HOSTNAME/" \
    "$SKETCH_PATH" > "$TEMP_SKETCH"

# 作成した一時スケッチファイルの内容を表示
cat "$TEMP_SKETCH"
echo "replace success => $TEMP_SKETCH"
echo "now compiling..."

# コンパイルとアップロード
arduino-cli compile --fqbn "$BOARD_NAME" "$TEMP_SKETCH"
if [ $? -eq 0 ]; then
    echo "Compilation successful, uploading..."
    arduino-cli upload -p /dev/ttyUSB0 --fqbn "$BOARD_NAME" "$TEMP_SKETCH"

    if [ $? -eq 0 ]; then
        echo "Upload successful!"
        stty -F /dev/ttyUSB0 $SERIAL_BAND
        cat /dev/ttyUSB0
    else
        echo "Upload failed"
    fi
else
    echo "Compilation failed"
fi

# 一時ファイルを削除
rm "$TEMP_SKETCH"
