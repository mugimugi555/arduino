import serial

# シリアルポートを指定します（例: '/dev/ttyUSB0'）
serial_port = '/dev/ttyUSB0'
baud_rate = 115200

with serial.Serial(serial_port, baud_rate, timeout=1) as ser:
    # ESP32にコマンドを送信
    ser.write(b'SEND_FILE\n')

    # ファイルを受信
    with open('odb_data.csv', 'wb') as f:
        while True:
            data = ser.read(1024)  # 1024バイトずつ読み取る
            if not data:
                break  # データがなくなったら終了
            f.write(data)
