#!/bin/bash

# bash upload_esp01_web.sh web_bme280/web_bme280.ino wifissid wifipasswd hostname

# 引数の確認
if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <sketch_path> <wifi_ssid> <wifi_password> <hostname>"
    exit 1
fi

# 引数の設定
SKETCH_PATH=$1      # Arduinoスケッチファイルのパス
WIFI_SSID=$2        # WiFi SSID
WIFI_PASSWORD=$3    # WiFi パスワード
HOSTNAME=$4         # ESP01のホスト名

# ESP01用のボード設定
BOARD_NAME=esp8266:esp8266:generic
SERIAL_BAND=115200
SERIAL_PORT=/dev/ttyUSB0

# ランダムなフォルダを作成
RANDOM_NAME=$(mktemp -u XXXXX)  # ランダムな名前を生成
RANDOM_DIR="/tmp/$RANDOM_NAME"
mkdir "$RANDOM_DIR"              # フォルダを作成
TEMP_SKETCH="$RANDOM_DIR/$RANDOM_NAME.ino"  # 同じ名前のファイルを作成
echo "Temporary sketch directory: $RANDOM_DIR"

# スケッチ内のWIFISSID、WIFIPASSWD、HOSTNAMEの置き換え
sed -e "s/WIFISSID/$WIFI_SSID/" \
    -e "s/WIFIPASSWD/$WIFI_PASSWORD/" \
    -e "s/HOSTNAME/$HOSTNAME/" \
    "$SKETCH_PATH" > "$TEMP_SKETCH"

# 作成した一時スケッチファイルの内容を表示
#cat "$TEMP_SKETCH"
echo "replace success => $TEMP_SKETCH"
echo "now compiling..."

# コンパイルとアップロード
arduino-cli compile --fqbn "$BOARD_NAME" "$TEMP_SKETCH"
if [ $? -eq 0 ]; then

    echo "Compilation successful, uploading..."
    arduino-cli upload -p "$SERIAL_PORT" --fqbn "$BOARD_NAME" "$TEMP_SKETCH"

    if [ $? -eq 0 ]; then
        # アップロード成功時に/dev/ttyUSB0の値を出力
        echo "Upload successful!"
        #echo "start serial monitor [ exit : Ctrl + A X ]"
        #picocom -b "$SERIAL_BAND" "$SERIAL_PORT"
    else
        echo "Upload failed"
    fi

else
    echo "Compilation failed"
fi

# 一時ファイルを削除
rm "$TEMP_SKETCH"
rmdir "$RANDOM_DIR"
