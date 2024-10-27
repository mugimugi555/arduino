#!/bin/bash

# YouTubeの動画URL
VIDEO_URL="https://www.youtube.com/watch?v=FtutLA63Cp8"

# 一時的なファイル名
VIDEO_FILE="video.mp4"
IMAGE_PREFIX="frame"
OUTPUT_DIR="frames"
JSON_FILE="frames_info.json"

# ディレクトリの作成
mkdir -p "$OUTPUT_DIR"

# 動画をダウンロード
yt-dlp -f best -o "$VIDEO_FILE" "$VIDEO_URL"

# フレームを抽出
ffmpeg -i "$VIDEO_FILE" -vf "fps=1" "$OUTPUT_DIR/$IMAGE_PREFIX%03d.bmp"

# 画像ファイルの枚数とファイル名を取得
IMAGE_FILES=("$OUTPUT_DIR/$IMAGE_PREFIX"*.bmp)
IMAGE_COUNT=${#IMAGE_FILES[@]}

# JSONファイルを作成
echo "{" > "$JSON_FILE"
echo "  \"image_count\": $IMAGE_COUNT," >> "$JSON_FILE"
echo "  \"image_files\": [" >> "$JSON_FILE"

for ((i = 0; i < IMAGE_COUNT; i++)); do
    FILE_NAME=$(basename "${IMAGE_FILES[$i]}")
    if [ $i -eq $((IMAGE_COUNT - 1)) ]; then
        echo "    \"$FILE_NAME\"" >> "$JSON_FILE"
    else
        echo "    \"$FILE_NAME\"," >> "$JSON_FILE"
    fi
done

echo "  ]" >> "$JSON_FILE"
echo "}" >> "$JSON_FILE"

# 完了メッセージ
echo "JSON file created: $JSON_FILE"
