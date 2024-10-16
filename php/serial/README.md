# Install Serialport Library
````bash
sudo apt install -y php php-xml php-bcmath php-mbstring php-xml php-zip php-curl ;
php -r "copy ( 'https://getcomposer.org/installer', 'composer-setup.php' ) ;" ;
sudo php composer-setup.php --install-dir=/usr/local/bin --filename=composer ;
composer -v ;
composer require "lepiaf/serialport" ;
php read_serial.php ;
````
