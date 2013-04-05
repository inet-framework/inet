set breakpoint pending on
# ## for Windows (separator is ';')
exec-file c:/omnetpp/bin/opp_run
set args -l e:/Users/kks/Documents/inet-hnrl/src/inet -n e:/Users/kks/Documents/inet-hnrl/examples;e:/Users/kks/Documents/inet-hnrl/src -u Cmdenv -f Video.ini -c N16_n1_vhigh_N1 -r 0
## for Linux (separator is ':')
#exec-file /home/kks/omnetpp/bin/opp_run
#set args -l /home/kks/tools/omnetpp/inet-hnrl/src/inet -n /home/kks/tools/omnetpp/inet-hnrl/examples:/home/kks/tools/omnetpp/inet-hnrl/src -u Cmdenv -f Video.ini -c N16_n1_vhigh_N1 -r 0
tbreak main
tbreak UDPVideoStreamSvrWithTrace3::initialize
#display state->snd_nxt
#display state->snd_una
#display state->snd_mss
#display state->snd_max
#display bytes
run
