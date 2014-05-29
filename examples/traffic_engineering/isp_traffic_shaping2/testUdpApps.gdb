set breakpoint pending on
exec-file ../../../../omnetpp-4.4.1/bin/opp_run
# ## for Windows (separator is ';')
# set args -l ../../../src/inet -n ../../../examples;../../../src -u Cmdenv -f testUdpApps.ini -r 0
## for Linux (separator is ':')
set args -l ../../../src/inet -n ../../../examples:../../../src -u Cmdenv -f testUdpApps.ini -c Test3 -r 0
tbreak main
tbreak UDPBurstApp::handleMessage
tbreak DropTailVLANTBFQueue::initialize
tbreak DropTailVLANTBFQueue::handleMessage
tbreak DropTailVLANTBFQueue::isConformed
#tbreak BurstMeter::handleMessage
# display messageLength
# display payloadLength
# display numSent
# display currentTime
run
