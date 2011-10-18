set breakpoint pending on
exec-file /home/kks/inet-hnrl/etc/omnetpp-extension/tests/distrib_test/out/gcc-debug/distrib_test
set args -l /home/kks/tools/omnetpp/inet-hnrl/src/inet -n /home/kks/tools/omnetpp/inet-hnrl/etc/omnetpp-extension/tests/distrib_test:/home/kks/tools/omnetpp/inet-hnrl/src -f omnetpp.ini -u Cmdenv -c Test9
tbreak main
break SampleGenerator::initialize
break SampleGenerator::handleMessage
#display bytes
run
