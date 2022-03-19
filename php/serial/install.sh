#!/usr/bin/bash

# install php
sudo apt install -y php php-xml php-bcmath php-mbstring php-xml php-zip php-curl ;

# install composer
php -r "copy ( 'https://getcomposer.org/installer', 'composer-setup.php' ) ;" ;
sudo php composer-setup.php --install-dir=/usr/local/bin --filename=composer ;
composer -v ;

# install php serial
cd ;
composer require "lepiaf/serialport" ;
