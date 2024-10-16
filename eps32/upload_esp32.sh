#!/bin/bash

# 引数の確認
if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <sketch_path> <wifi_ssid> <wifi_password> <hostname>"
    exit 1
fi

SKETCH_PATH=$1      # Arduinoスケッチファイルのパス
WIFI_SSID=$2        # WiFi SSID
WIFI_PASSWORD=$3    # WiFi パスワード
HOSTNAME=$4         # ESP8266のホスト名

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

echo "$TEMP_SKETCH"

# コンパイルとアップロード
arduino-cli compile --fqbn esp32:esp32:esp32 "$TEMP_SKETCH"
if [ $? -eq 0 ]; then
    echo "Compilation successful, uploading..."
    arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 "$TEMP_SKETCH"
else
    echo "Compilation failed"
fi

# 一時ファイルを削除
rm "$TEMP_SKETCH"
