# Install Arduino CLI
````bash
cd
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
echo 'export PATH=$PATH:$HOME/bin' >> ~/.bashrc
source ~/.bashrc
````
#
````bash
arduino-cli config init
arduino-cli core update-index
arduino-cli core list
````
# Install ESP01 board
````bash
arduino-cli config set board_manager.additional_urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32
arduino-cli core list
````
# Install Library
````bash
arduino-cli lib install "Adafruit BME280 Library"
arduino-cli lib install "Adafruit BusIO"
arduino-cli lib install "Adafruit Unified Sensor"
arduino-cli lib install "DHT sensor library"
````
# Compile and Upload command
````bash
bash upload_esp01.sh web_bmp180/web_bmp180.ino wifissid wifipasswd hostname
````
