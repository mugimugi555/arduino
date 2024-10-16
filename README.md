# install Arduino IDE
````bash
sudo add-apt-repository universe
sudo apt install -y libfuse2
#SUBSYSTEMS=="usb", ATTRS{idVendor}=="2341", GROUP="plugdev", MODE="0666"
echo 'KERNEL=="tty[A-Z]*[0-9]|pppox[0-9]*|ircomm[0-9]*|noz[0-9]*|rfcomm[0-9]*", GROUP="dialout", MODE="0666"' | sudo tee /etc/udev/rules.d/99-arduino.rules ;

cd
wget https://downloads.arduino.cc/arduino-ide/arduino-ide_2.3.3_Linux_64bit.AppImage
chmod +x arduino-ide_2.3.3_Linux_64bit.AppImage
./arduino-ide_2.3.3_Linux_64bit.AppImage --no-sandbox
````
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
# Install ESP32 Board
````bash
arduino-cli config set board_manager.additional_urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32
arduino-cli core list
````

# Install ESP8266 Board
````bash
arduino-cli config set board_manager.additional_urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core update-index
arduino-cli core install esp8266:esp8266
arduino-cli core list
````
