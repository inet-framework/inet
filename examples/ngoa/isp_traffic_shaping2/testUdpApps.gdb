set breakpoint pending on
exec-file ../../../../omnetpp-4.2.2/bin/opp_run
#set args -l /e/Tools/omnetpp/inet-hnrl/src/inet -n /e/Tools/omnetpp/inet-hnrl/examples:/e/Tools/omnetpp/inet-hnrl/src -u Cmdenv -f testUdpApps.ini -r 0
set args -l ../../../src/inet -n ../../../examples;../../../src -u Cmdenv -f testUdpApps.ini -r 0
tbreak main
tbreak UDPBurstApp::sendPacket
# display messageLength
# display payloadLength
# display numSent
# display currentTime
run
