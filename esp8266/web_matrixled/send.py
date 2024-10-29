# pip install pyserial
# python3 send_serial.py "送信するメッセージ"

import sys
import serial
import time

# シリアルポートの設定（ESP8266のポートに変更してください）
serial_port = '/dev/ttyUSB0'
baud_rate = 115200

# 引数からメッセージを取得（引数がない場合はデフォルトメッセージ）
message = sys.argv[1] if len(sys.argv) > 1 else "デフォルトのメッセージ"

# シリアルポートをオープン
ser = serial.Serial(serial_port, baud_rate, timeout=1)
time.sleep(2)  # 通信の安定化のために少し待機

# メッセージを送信
ser.write((message + '\n').encode())
print(f"メッセージを送信しました: {message}")

# シリアルポートをクローズ
ser.close()

