#Install Arduino CLI
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
````
#Install ESP32 board
````bash
arduino-cli config set board_manager.additional_urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32
````
#
````bash
arduino-cli core list
````

#Compile and Upload command
````bash
bash uploadesp32.sh web_bmp180/web_bmp180.ino wifissid wifipasswd hostname
````
