@echo off
rem
rem usage: runtest [<testfile>...]
rem without args, runs all *.test files in the current directory
rem

set TESTFILES=%*
if "x%TESTFILES%" == "x" set TESTFILES=*.test
mkdir work 2>nul
xcopy /i /y lib work\lib
: del work\work.exe 2>nul

call opp_test -g -v %TESTFILES% || goto end

echo.
set EXTRA_INCLUDES=-I..\..\..\src\applications -I..\..\..\src\base -I..\..\..\src\battery -I..\..\..\src\linklayer -I..\..\..\src\mobility -I..\..\..\src\networklayer -I..\..\..\src\nodes -I..\..\..\src\transport -I..\..\..\src\util -I..\..\..\src\world -I..\..\..\src\applications\ethernet -I..\..\..\src\applications\generic -I..\..\..\src\applications\httptools -I..\..\..\src\applications\pingapp -I..\..\..\src\applications\rtpapp -I..\..\..\src\applications\sctpapp -I..\..\..\src\applications\tcpapp -I..\..\..\src\applications\udpapp -I..\..\..\src\applications\voiptool -I..\..\..\src\battery\models -I..\..\..\src\linklayer\contract -I..\..\..\src\linklayer\ethernet -I..\..\..\src\linklayer\ext -I..\..\..\src\linklayer\ieee80211 -I..\..\..\src\linklayer\ieee80211mesh -I..\..\..\src\linklayer\mf80211 -I..\..\..\src\linklayer\mfcore -I..\..\..\src\linklayer\ppp -I..\..\..\src\linklayer\radio -I..\..\..\src\linklayer\ethernet\switch -I..\..\..\src\linklayer\ieee80211\mac -I..\..\..\src\linklayer\ieee80211\mgmt -I..\..\..\src\linklayer\ieee80211\radio -I..\..\..\src\linklayer\ieee80211\radio\errormodel -I..\..\..\src\linklayer\ieee80211mesh\mgmt -I..\..\..\src\linklayer\mf80211\core -I..\..\..\src\linklayer\mf80211\macLayer -I..\..\..\src\linklayer\mf80211\phyLayer -I..\..\..\src\linklayer\mf80211\phyLayer\decider -I..\..\..\src\linklayer\mf80211\phyLayer\snrEval -I..\..\..\src\linklayer\radio\propagation -I..\..\..\src\mobility\models -I..\..\..\src\networklayer\arp -I..\..\..\src\networklayer\autorouting -I..\..\..\src\networklayer\bgpv4 -I..\..\..\src\networklayer\common -I..\..\..\src\networklayer\contract -I..\..\..\src\networklayer\extras -I..\..\..\src\networklayer\icmpv6 -I..\..\..\src\networklayer\ipv4 -I..\..\..\src\networklayer\ipv6 -I..\..\..\src\networklayer\ipv6tunneling -I..\..\..\src\networklayer\ldp -I..\..\..\src\networklayer\manetrouting -I..\..\..\src\networklayer\mpls -I..\..\..\src\networklayer\ospfv2 -I..\..\..\src\networklayer\queue -I..\..\..\src\networklayer\rsvp_te -I..\..\..\src\networklayer\ted -I..\..\..\src\networklayer\xmipv6 -I..\..\..\src\networklayer\autorouting\ipv4 -I..\..\..\src\networklayer\autorouting\ipv6 -I..\..\..\src\networklayer\bgpv4\BGPMessage -I..\..\..\src\networklayer\manetrouting\aodv -I..\..\..\src\networklayer\manetrouting\base -I..\..\..\src\networklayer\manetrouting\batman -I..\..\..\src\networklayer\manetrouting\dsdv -I..\..\..\src\networklayer\manetrouting\dsr -I..\..\..\src\networklayer\manetrouting\dymo -I..\..\..\src\networklayer\manetrouting\dymo_fau -I..\..\..\src\networklayer\manetrouting\olsr -I..\..\..\src\networklayer\manetrouting\aodv\aodv-uu -I..\..\..\src\networklayer\manetrouting\dsr\dsr-uu -I..\..\..\src\networklayer\manetrouting\dymo\dymoum -I..\..\..\src\networklayer\ospfv2\interface -I..\..\..\src\networklayer\ospfv2\messagehandler -I..\..\..\src\networklayer\ospfv2\neighbor -I..\..\..\src\networklayer\ospfv2\router -I..\..\..\src\nodes\bgp -I..\..\..\src\nodes\ethernet -I..\..\..\src\nodes\httptools -I..\..\..\src\nodes\inet -I..\..\..\src\nodes\ipv6 -I..\..\..\src\nodes\mf80211 -I..\..\..\src\nodes\mpls -I..\..\..\src\nodes\ospfv2 -I..\..\..\src\nodes\wireless -I..\..\..\src\nodes\xmipv6 -I..\..\..\src\transport\contract -I..\..\..\src\transport\rtp -I..\..\..\src\transport\sctp -I..\..\..\src\transport\tcp -I..\..\..\src\transport\tcp_common -I..\..\..\src\transport\tcp_lwip -I..\..\..\src\transport\tcp_nsc -I..\..\..\src\transport\udp -I..\..\..\src\transport\rtp\profiles -I..\..\..\src\transport\rtp\profiles\avprofile -I..\..\..\src\transport\tcp\flavours -I..\..\..\src\transport\tcp\queues -I..\..\..\src\transport\tcp_lwip\include -I..\..\..\src\transport\tcp_lwip\lwip -I..\..\..\src\transport\tcp_lwip\omnet -I..\..\..\src\transport\tcp_lwip\queues -I..\..\..\src\transport\tcp_lwip\include\arch -I..\..\..\src\transport\tcp_lwip\lwip\core -I..\..\..\src\transport\tcp_lwip\lwip\include -I..\..\..\src\transport\tcp_lwip\lwip\include\arch -I..\..\..\src\transport\tcp_lwip\lwip\include\ipv4 -I..\..\..\src\transport\tcp_lwip\lwip\include\ipv6 -I..\..\..\src\transport\tcp_lwip\lwip\include\lwip -I..\..\..\src\transport\tcp_lwip\lwip\include\netif -I..\..\..\src\transport\tcp_lwip\lwip\include\ipv4\lwip -I..\..\..\src\transport\tcp_lwip\lwip\include\ipv6\lwip -I..\..\..\src\transport\tcp_nsc\queues -I..\..\..\src\util\headerserializers -I..\..\..\src\util\headerserializers\headers -I..\..\..\src\util\headerserializers\ipv4 -I..\..\..\src\util\headerserializers\sctp -I..\..\..\src\util\headerserializers\tcp -I..\..\..\src\util\headerserializers\udp -I..\..\..\src\util\headerserializers\ipv4\headers -I..\..\..\src\util\headerserializers\sctp\headers -I..\..\..\src\util\headerserializers\tcp\headers -I..\..\..\src\util\headerserializers\udp\headers -I..\..\..\src\world\annotations -I..\..\..\src\world\httptools -I..\..\..\src\world\obstacles -I..\..\..\src\world\powercontrol -I..\..\..\src\world\radio -I..\..\..\src\world\scenario
cd work || goto end
call opp_nmakemake -f --deep -linet -L../../../src -P . --no-deep-includes %EXTRA_INCLUDES%
nmake -f makefile.vc || cd .. && goto end
cd .. || goto end

echo.
path %~dp0\..\..\src;%PATH%
call opp_test -r %OPT% -v %TESTFILES% || goto end

echo.
echo Results can be found in work/

:end