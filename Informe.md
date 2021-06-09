# TP3

## Instalación

- Correr el siguiente comando `install.sh`.

## Uso

- Mapear en el **/etc/hosts** del cliente el dominio **www.tp3.com**
a la ip que corresponda.  
- Se puede consumir la API mediante la coleccion de _Postman_ 
adjuntada.

## Desarrollo

### Servicios

- Se armó una función variádica para loggear.
- Se usó el comando `useradd` para los cada nuevos usuarios.
  - En un principio se pasaba la contraseña como texto plano.
  - Hasta que descubrí que no podía logear usando `ssh` con
  los usuarios nuevos.
  - Por lo visto ssh no funcionaba con la contraseña en texto
  plano.
  - Se encriptó con el comando `openssl`, y anduvo.
  - No se encriptó a mano con la función `crypt` porque no 
  sabía que ponerle en el argumento salt.
 

#### Descarga

- Se partió de los ejemplos de la documentación de ulfius,
y se hizo sin más problemas.
- Al pedir un archivo, se agarra al primero del directorio
que corresponde a ese producto, en ese día y a esa hora.

#### Usuarios

- Se partió de los ejemplos de la documentación de ulfius,
y se hizo sin más problemas.
- Para conseguir la lista de usuarios, se recorrió el 
archivo /etc/passwd, que tiene una línea por cada usuario,
describiendo su cuenta.

### Proxy inverso

- Para implementar el web server con su proxy inverso, se 
usó **nginx**.
- Se redireccionan las request de las api que llegan al puerto 80, 
puerto por defecto de HTTP, a la url del servicio correspondiente, 
en el puerto de corresponde.
- Se usó la autenticación por HTTP, para lo cual se usó el comando
`htpasswd` para crear el archivo con los pares de usuarios y hashes.
- Se copia dicho archivo, llamado `.htpasswd` en /etc/apache2, y 
al querer acceder a los servicios del servidor, ya sea para consumir
la API o para descargar archivos, hay que autenticarse.

### Gestión de Servicios

- Para gestionar los servicios se usó `systemD`.
- Se creó un usuario por cada servicio a ejecutar, para no tener que
ejecutar todo como sudo.
- Para ejecutar el comando `useradd`, se le dio permiso al usuario
**www-users** encargado del servicio de usuarios para ejecutarlo como
sudo.

