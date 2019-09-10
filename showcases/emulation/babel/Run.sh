# start babel routing daemons
sudo ip netns exec host0 babeld -I babel0.pid -S babel-state0 -r -w -h 2 -M 0 -C 'reflect-kernel-metric true' -C 'interface tap0 channel 1 link-quality false' &
sudo ip netns exec host1 babeld -I babel1.pid -S babel-state1 -r -w -h 2 -M 0 -C 'reflect-kernel-metric true' -C 'interface tap1 channel 1 link-quality false' &
sudo ip netns exec host2 babeld -I babel2.pid -S babel-state2 -r -w -h 2 -M 0 -C 'reflect-kernel-metric true' -C 'interface tap2 channel 1 link-quality false' &

# start ping loop to avoid exiting early and to wait for the simulation to start
sudo ip netns exec host0 bash -c 'while ! ping 192.168.4.1; do sleep 1; done' &

# start simulation and wait for the user to exit
inet

# stop ping loop
ps -fe | grep -v awk | awk '/ping 192.168.4.1/ {print $2}' | xargs sudo kill

# stop babel routing deamons
sudo killall babeld

