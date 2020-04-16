# destroy TAP interfaces
sudo ip netns exec host0 ip tuntap del mode tap dev tap0
sudo ip netns exec host1 ip tuntap del mode tap dev tap1
sudo ip netns exec host2 ip tuntap del mode tap dev tap2

# destroy network namespaces
sudo ip netns del host0
sudo ip netns del host1
sudo ip netns del host2

