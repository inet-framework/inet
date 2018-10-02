# create TAP interfaces
sudo ip tuntap add mode tap dev tapa

# assign IP addresses to interfaces
sudo ip addr add 192.168.2.1/24 dev tapa

# bring up all interfaces
sudo ip link set dev tapa up
