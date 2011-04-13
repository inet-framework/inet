set breakpoint pending on
exec-file /home/kks/omnetpp/bin/opp_run
set args -l /home/kks/inet-hnrl/src/inet -n /home/kks/inet-hnrl/examples:/home/kks/inet-hnrl/src -f ./Reference.ini -u Cmdenv -c Parsim_N2 -r 0
tbreak main
tbreak SimTime::overflowAdding
tbreak IPAddressResolver::interfaceTableOf
#display state->snd_nxt
#display state->snd_una
#display state->snd_mss
#display state->snd_max
#display bytes
run
