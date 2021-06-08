# Instalar ulfius
sudo apt-get install libulfius-dev

sudo make

# Cargar configuración para el proxy reverso
sudo cp ./nginx/sites-available-proxy /etc/nginx/sites-available/
sudo nginx -s reload
# setear /etc/hosts en el cliente


# copiar proxy en /etc/nginx/sites-available/default
sudo cp nginx/sites-available/proxy /etc/nginx/sites-available/default
# copiar .htpasswd en /etc/apache2
sudo cp nginx/.htpasswd /etc/apache2/.htpasswd

# copiar *.service en /etc/systemd/system/
sudo cp services/goes.service /etc/systemd/system/goes.service
sudo systemctl start goes
sudo cp services/users.service /etc/systemd/system/users.service
sudo systemctl start users
