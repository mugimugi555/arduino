#!/bin/bash

# Set variables
VIDEO_URL="https://www.youtube.com/watch?v=FtutLA63Cp8"  # Video URL
IMAGE_DIR="./images"  # Directory for images
VIDEO_FILE="video.mp4"  # Downloaded video filename
SERIAL_PORT="/dev/ttyUSB0"  # Serial port
BAUD_RATE=115200  # Baud rate for serial communication
WIDTH=128  # Image width
HEIGHT=32  # Image height

# Create the image directory
mkdir -p $IMAGE_DIR

# Download the video if it hasn't been downloaded yet
if [ ! -f "$VIDEO_FILE" ]; then
  echo "動画をダウンロードします..."
  yt-dlp -o "$VIDEO_FILE" "$VIDEO_URL"
fi

# Extract frames from the video using ffmpeg
echo "動画から静止画像pngを出力します。"
ffmpeg -y -loglevel error -i "$VIDEO_FILE" -vf "fps=1" "$IMAGE_DIR/frame_%03d.png"

# Check if any frames were successfully extracted
if ls "$IMAGE_DIR/frame_"*.png 1> /dev/null 2>&1; then
  echo "静止画pngの出力が完了しました。"
else
  echo "No images found after extraction. Please check the video."
  exit 1
fi

# Resize and convert images to monochrome BMP format
echo "静止画をモノクロのBMP画像に変換します。"
for img in "$IMAGE_DIR/frame_"*.png; do
  echo -ne "画像の変換 $img...\r"
  # Resize to OLED screen dimensions and convert to monochrome BMP
  ffmpeg -y -loglevel error -i "$img" -vf "scale=${WIDTH}:${HEIGHT},format=monob" "${img%.png}.bmp"
done

# Open the serial port for communication with the specified baud rate
stty -F $SERIAL_PORT $BAUD_RATE cs8 -cstopb -parenb

# Send the BMP files without headers
echo "========================================="
echo "モノクロBMP画像をArduinoへシリアル送信します..."
echo "========================================="
for img in "$IMAGE_DIR/frame_"*.bmp; do

  echo -ne "画像の送信 $img...\r"

  # BMPファイルのヘッダーのオフセット値の取得
  offset=$(xxd -s 10 -l 4 -u -ps "$img" | xxd -r -ps | od -An -t u4)
  dd if="$img" bs="$offset" skip=1 status=none  | cat > $SERIAL_PORT

  sleep 0.1

done

echo "画像の送信完了"
