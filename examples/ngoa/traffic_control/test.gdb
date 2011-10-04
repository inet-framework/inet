set breakpoint pending on
exec-file /home/kks/omnetpp/bin/opp_run
set args -l /home/kks/tools/omnetpp/inet-hnrl/src/inet -n /home/kks/tools/omnetpp/inet-hnrl/examples:/home/kks/tools/omnetpp/inet-hnrl/src -f test.ini -u Cmdenv -c N1_n1_nh1_nf0_nv0 
tbreak main
#tbreak UDP::bind
tbreak DropTailTBFQueue::handleMessage
#display state->snd_nxt
#display state->snd_una
#display state->snd_mss
#display state->snd_max
#display bytes
run
