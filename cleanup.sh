GROUP_NAME=SERVER_USERS

users=$(members $GROUP_NAME)
for uid in $users
do
  sudo deluser $uid
done

sudo deluser www-goes
sudo deluser www-users
sudo systemctl stop users
sudo systemctl stop goes
sudo rm /var/log/tp3/*
