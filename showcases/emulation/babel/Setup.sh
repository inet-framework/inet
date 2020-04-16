# create network namespaces
sudo ip netns add host0
sudo ip netns add host1
sudo ip netns add host2

# create TAP interfaces
sudo ip netns exec host0 ip tuntap add mode tap dev tap0
sudo ip netns exec host1 ip tuntap add mode tap dev tap1
sudo ip netns exec host2 ip tuntap add mode tap dev tap2

# bring up TAP interfaces
sudo ip netns exec host0 ip link set dev tap0 up
sudo ip netns exec host1 ip link set dev tap1 up
sudo ip netns exec host2 ip link set dev tap2 up

# assign IP addresses to interface
sudo ip netns exec host0 ip addr add 192.168.2.1/32 dev tap0
sudo ip netns exec host1 ip addr add 192.168.3.1/32 dev tap1
sudo ip netns exec host2 ip addr add 192.168.4.1/32 dev tap2

