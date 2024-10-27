#!/bin/bash

# 変数の設定
VIDEO_URL="https://www.youtube.com/watch?v=FtutLA63Cp8"  # ここに動画のURLを設定
IMAGE_DIR="./images"  # 画像を保存するディレクトリ
BMP_FILE="image.bmp"  # 出力するBMPファイル名
SERIAL_PORT="/dev/ttyUSB0"  # シリアルポートを設定
BAUD_RATE=115200

# 画像を保存するディレクトリを作成
mkdir -p $IMAGE_DIR

# YouTube動画をダウンロードして画像を切り出す
yt-dlp --skip-download --hls-prefer-native --output "$IMAGE_DIR/frame_%03d.png" "$VIDEO_URL"

# 画像をモノクロBMPに変換
for img in "$IMAGE_DIR/frame_"*.png; do
  convert "$img" -colors 2 -depth 1 "$img.bmp"
  mv "$img.bmp" "$img"  # 元の画像を上書き
done

# シリアルポートを開いてESP8266に画像を送信
stty -F $SERIAL_PORT $BAUD_RATE cs8 -cstopb -parenb

for img in "$IMAGE_DIR/frame_"*.png; do
  # BMPファイルをバイナリ形式でESP8266に送信
  echo "Sending $img..."
  cat "$img" > $SERIAL_PORT
  sleep 1  # 次の画像を送信する前に少し待つ
done

echo "Image transfer complete."
