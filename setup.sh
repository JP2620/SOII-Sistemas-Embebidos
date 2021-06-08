# Cargar configuración para el proxy reverso
sudo cp ./nginx/sites-available-proxy /etc/nginx/sites-available/
sudo nginx -s reload

# copiar proxy en /etc/nginx/sites-available/default
# copiar .htpasswd en /etc/apache2

# copiar *.service en /etc/systemd/system/