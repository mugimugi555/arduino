#!/bin/bash

# 使用方法: ./send_image.sh input.png
# 引数で指定された画像ファイル
input_image="$1"
output_pgm="output.pgm"
serial_device="/dev/ttyUSB0"
baud_rate=9600

# ImageMagickで画像を5x8にリサイズし、モノクロ二値化してPGM形式に変換
convert "$input_image" -resize 5x8\! -threshold 50% -depth 1 "$output_pgm"

# シリアル接続を設定
stty -F "$serial_device" "$baud_rate" cs8 -cstopb -parenb

# PGMファイルを読み込み、画像データ部分を整形してシリアルに送信
{
  while IFS= read -r line; do
    # コメント行とヘッダー行をスキップ（# または 'P2' で始まる行）
    if [[ "$line" =~ ^# ]] || [[ "$line" =~ ^P2 ]]; then
      continue
    fi
    # 画像データ行（スペースを削除して連結）
    binary_line=$(echo "$line" | tr -d ' ')
    echo -n "$binary_line" > "$serial_device"
    echo "Sent: $binary_line" # 送信内容を確認用に出力
  done < "$output_pgm"
} || {
  echo "Error: Could not send data to $serial_device."
}

echo "データの送信が完了しました。"
