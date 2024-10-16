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
# Install ESP8266 Board
````bash
arduino-cli config set board_manager.additional_urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core update-index
arduino-cli core install esp8266:esp8266
arduino-cli core list
````
# Compile and Upload command
````bash
bash upload_esp8266.sh web_dht11/web_dht11.ino wifissid wifipasswd hostname
````
