set breakpoint pending on
exec-file ../../../../omnetpp-4.3/bin/opp_run
# ## for Windows (separator is ';')
# set args -l ../../../src/inet -n ../../../examples;../../../src -u Cmdenv -f Mixed.ini -c drr -r 0
## for Linux (separator is ':')
set args -l ../../../src/inet -n ../../../examples:../../../src -u Cmdenv -f Mixed.ini -c drr -r 0
tbreak DRRVLANQueue2::initialize
tbreak DRRVLANQueue2::handleMessage
break DRRVLANQueue2::requestPacket
break DRRVLANQueue2.cc:201
break DRRVLANQueue2.cc:218
break DRRVLANQueue2.cc:256
break DRRVLANQueue2.cc:262
# display messageLength
# display payloadLength
# display numSent
# display currentTime
run
