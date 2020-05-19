# create two virtual ethernet links: a <--> b and c <--> d
sudo ip link add vetha type veth peer name vethb
sudo ip link add vethc type veth peer name vethd

# a <--> b link uses 192.168.2.x addresses and c <--> d link uses 192.168.3.x addresses
sudo ip addr add 192.168.2.20 dev vetha
sudo ip addr add 192.168.3.20 dev vethd

# bring up all interfaces
sudo ip link set vetha up
sudo ip link set vethb up
sudo ip link set vethc up
sudo ip link set vethd up

# add routes for new links
sudo route add -net 192.168.2.0 netmask 255.255.255.0 dev vetha
sudo route add -net 192.168.3.0 netmask 255.255.255.0 dev vethd
