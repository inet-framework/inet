set breakpoint pending on
exec-file ../../../../omnetpp-4.3/bin/opp_run
# ## for Windows (separator is ';')
# set args -l ../../../src/inet -n ../../../examples;../../../src -u Cmdenv -f Shared.ini -c csfq-tbm_debug_dynamic -r 0
## for Linux (separator is ':')
set args -l ../../../src/inet -n ../../../examples:../../../src -u Cmdenv -f Shared.ini -c csfq-tbm_debug_dynamic -r 0
tbreak main
tbreak CSFQVLANQueue::initialize
tbreak CSFQVLANQueue::estimateAlpha
# display messageLength
# display payloadLength
# display numSent
# display currentTime
run
