set breakpoint pending on
exec-file /home/kks/omnetpp/bin/opp_run
set args -l /home/kks/tools/omnetpp/inet-hnrl/src/inet -n /home/kks/tools/omnetpp/inet-hnrl/examples:/home/kks/tools/omnetpp/inet-hnrl/src -f omnetpp.ini -u Cmdenv -c Test1 -r 0 
tbreak main
tbreak SampleGenerator::initialize
tbreak SampleGenerator::handleMessage
tbreak PercentileRecorder::finish
#display bytes
run
