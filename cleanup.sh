GROUP_NAME=SERVER_USERS

users=$(members $GROUP_NAME)
for uid in $users
do
  sudo deluser $uid
done
