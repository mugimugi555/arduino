
# https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
# https://solectroshop.com/es/content/125-tutorial-para-la-placa-wemos-d1-esp32-r32-wroom-32-wifi-y-bluetooth
# https://qiita.com/ketaro-m/items/edd40ba08ff61c7bc0e6

#
cd ;
wget https://downloads.arduino.cc/arduino-1.8.19-linux64.tar.xz ;
tar xvf arduino-1.8.19-linux64.tar.xz ;
cd arduino-1.8.19-linux64 ;
sudo ./install ;

#
curl https://bootstrap.pypa.io/pip/2.7/get-pip.py --output get-pip.py ;
sudo python2 get-pip.py ;
python -m pip install pyserial ;

# sudo adduser $USER dialout
# sudo chmod 777 /dev/ttyUSB0

# 50-udev-default.rules
echo 'KERNEL=="tty[A-Z]*[0-9]|pppox[0-9]*|ircomm[0-9]*|noz[0-9]*|rfcomm[0-9]*", GROUP="dialout", MODE="0666"' | sudo tee /etc/udev/rules.d/50-udev-default.rules ;
