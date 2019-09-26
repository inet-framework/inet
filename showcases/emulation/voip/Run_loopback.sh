# remove delay+loss+corruption from loopback interface (just in case the script was interrupted previously)
sudo tc qdisc del dev lo root

# add delay+loss+corruption to loopback interface
sudo tc qdisc add dev lo root netem loss 1% corrupt 5% delay 10ms 1ms

# run the simulations
inet -u Cmdenv -f receiver.ini -c LoopbackReceiver &
sleep 1
inet -u Cmdenv -f sender.ini -c LoopbackSender
wait

# remove delay+loss+corruption from loopback interface
sudo tc qdisc del dev lo root


