# Instalar ulfius
sudo apt install -y libmicrohttpd-dev libjansson-dev libcurl4-gnutls-dev libgnutls28-dev libgcrypt20-dev libsystemd-dev zlib1g-dev
wget https://github.com/babelouest/ulfius/releases/download/v2.7.0/ulfius-dev-full_2.7.0_debian_buster_x86_64.tar.gz
tar xf ulfius-dev-full_2.7.0_debian_buster_x86_64.tar.gz
sudo dpkg -i *.deb
rm *.deb *.gz
sudo apt-get install awscli -y

sudo apt-get install build-essential -y
sudo make

# Crear carpeta para el log
sudo mkdir /var/log/tp3

# Crea usuario que correrá el servicio de descargas
sudo groupadd -g 1111 www-goes
sudo groupadd -g 1112 www-users
sudo useradd --gid www-goes --password $(openssl passwd -6 1234) www-users
sudo useradd --gid www-goes --password $(openssl passwd -6 1234) www-goes
sudo chown -R www-goes:www-goes /var/www/goes
sudo cp ./sudoers/* /etc/sudoers.d/

sudo groupadd -g 1113 www-tp3
sudo usermod -a -G www-tp3 www-users
sudo usermod -a -G www-tp3 www-goes
sudo touch /var/log/tp3/log.txt
sudo chgrp -R www-tp3 /var/log/tp3/
sudo chmod 777 -R /var/log/

# Crea root del servicio de descarga

sudo mkdir /var/www/goes
sudo groupadd -g 1234 SERVER_USERS

# Cargar configuración para el proxy reverso
sudo apt-get install nginx -y
sudo cp nginx/sites-available/proxy /etc/nginx/sites-available/
sudo mv /etc/nginx/sites-available/proxy /etc/nginx/sites-available/default
# Copiar .htpasswd en /etc/apache2
sudo apt-get install apache2-utils -y
sudo cp nginx/.htpasswd /etc/apache2/
# Cargar cambios
sudo nginx -s reload

# Copiar *.service en /etc/systemd/system/
sudo cp services/goes.service /etc/systemd/system/goes.service
sudo cp services/users.service /etc/systemd/system/users.service
sudo systemctl daemon-reload
sudo systemctl start goes
sudo systemctl start users