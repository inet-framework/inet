set breakpoint pending on
exec-file /home/kks/omnetpp/bin/opp_run
set args -l /home/kks/tools/omnetpp/inet-hnrl/src/inet -n /home/kks/tools/omnetpp/inet-hnrl/examples;/home/kks/tools/omnetpp/inet-hnrl/src -u Cmdenv -f testVideo.ini -c Client2 -r 0
tbreak main
#tbreak UDPVideoStreamCliWithTrace::initialize
tbreak UDPVideoStreamCliWithTrace2::initialize
#display state->snd_nxt
#display state->snd_una
#display state->snd_mss
#display state->snd_max
#display bytes
run
