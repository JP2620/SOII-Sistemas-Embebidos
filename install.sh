# Instalar ulfius
sudo apt install -y libmicrohttpd-dev libjansson-dev libcurl4-gnutls-dev libgnutls28-dev libgcrypt20-dev libsystemd-dev zlib1g-dev
wget https://github.com/babelouest/ulfius/releases/download/v2.7.0/ulfius-dev-full_2.7.0_debian_buster_x86_64.tar.gz
tar xf ulfius-dev-full_2.7.0_debian_buster_x86_64.tar.gz
sudo dpkg -i *.deb
rm *.deb

sudo apt-get install build-essential -y
sudo make

# Crear carpeta para el log
sudo mkdir /var/log/tp3

# Crea root del servicio de descarga
sudo apt-get install awscli -y
sudo mkdir /var/www/goes
sudo groupadd -g 1234 SERVER_USERS

# Cargar configuración para el proxy reverso
sudo apt-get install nginx -y
sudo cp nginx/sites-available/proxy /etc/nginx/sites-available/
sudo mv /etc/nginx/sites-available/proxy /etc/nginx/sites-available/default
# Copiar .htpasswd en /etc/apache2
sudo cp nginx/.htpasswd /etc/apache2/
# Cargar cambios
sudo nginx -s reload

# Copiar *.service en /etc/systemd/system/
sudo cp services/goes.service /etc/systemd/system/goes.service
sudo cp services/users.service /etc/systemd/system/users.service
sudo systemctl daemon-reload
sudo systemctl start goes
sudo systemctl start users