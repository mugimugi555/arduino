#!/usr/bin/bash

# wget https://raw.githubusercontent.com/mugimugi555/arduino/main/php/serial/install.sh && bash install.sh ;

# install php
sudo apt install -y php php-xml php-bcmath php-mbstring php-xml php-zip php-curl ;

# install composer
php -r "copy ( 'https://getcomposer.org/installer', 'composer-setup.php' ) ;" ;
sudo php composer-setup.php --install-dir=/usr/local/bin --filename=composer ;
composer -v ;

# install php serial
cd ;
composer require "lepiaf/serialport" ;

# set tttyUSB0 permission

# sudo adduser $USER dialout
# sudo chmod 777 /dev/ttyUSB0

# 50-udev-default.rules
echo 'KERNEL=="tty[A-Z]*[0-9]|pppox[0-9]*|ircomm[0-9]*|noz[0-9]*|rfcomm[0-9]*", GROUP="dialout", MODE="0666"' | sudo tee /etc/udev/rules.d/50-udev-default.rules ;

# do php serial
wget https://raw.githubusercontent.com/mugimugi555/arduino/main/php/serial/read_serial.php && php read_serial.php ;
