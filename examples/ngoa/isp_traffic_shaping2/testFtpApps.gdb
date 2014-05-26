set breakpoint pending on
exec-file ../../../../omnetpp-4.2.2/bin/opp_run
# ## for Windows (separator is ';')
# set args -l ../../../src/inet -n ../../../examples;../../../src -u Cmdenv -f testFtpApps.ini -c FtpClient -r 0
## for Linux (separator is ':')
set args -l ../../../src/inet -n ../../../examples:../../../src -u Cmdenv -f testFtpApps.ini -c HttpClient -r 0
tbreak main
tbreak FtpClientApp::initialize
tbreak FtpClientApp::sendRequest	
tbreak HttpClientApp::initialize
tbreak HttpClientApp::sendRequest
# display messageLength
# display payloadLength
# display numSent
# display currentTime
run
