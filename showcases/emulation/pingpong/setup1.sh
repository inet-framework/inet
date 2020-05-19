# create two virtual ethernet links: a <--> b
sudo ip link add vetha type veth peer name vethb

# a <--> b link uses 192.168.2.x addresses
sudo ip addr add 192.168.2.2 dev vethb

# bring up all interfaces
sudo ip link set vetha up
sudo ip link set vethb up

# add routes for new links
sudo route add -net 192.168.2.0 netmask 255.255.255.0 dev vethb

