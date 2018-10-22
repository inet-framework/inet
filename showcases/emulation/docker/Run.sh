sudo docker exec c0 babeld eth1 &
sudo docker exec c1 babeld eth1 &
sudo docker exec c2 babeld eth1 &

sudo docker exec c0 ping 192.168.4.2 &

inet

sudo docker exec c0 killall babeld
sudo docker exec c1 killall babeld
sudo docker exec c2 killall babeld

sudo docker exec c0 killall ping

