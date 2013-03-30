set breakpoint pending on
exec-file /home/kks/omnetpp/bin/opp_run
# ## for Windows (separator is ';')
# set args -l /home/kks/inet-hnrl/src/inet -n /home/kks/inet-hnrl/examples;/home/kks/inet-hnrl/src -f ./TestOfDedicatedAccess.ini -u Cmdenv -c Debug_4 -r 128
## for Linux (separator is ':')
set args -l /home/kks/inet-hnrl/src/inet -n /home/kks/inet-hnrl/examples:/home/kks/inet-hnrl/src -f ./TestOfDedicatedAccess.ini -u Cmdenv -c Debug_4 -r 128
tbreak main
tbreak TCPConnection::retransmitOneSegment
#display state->snd_nxt
#display state->snd_una
#display state->snd_mss
#display state->snd_max
#display bytes
run
