call ..\omnetpp\vcvars32.bat
set root=D:\home\IPSuite-pcvs
set MAKEMAKE=cmd /c d:\home\omnetpp\bin\opp_nmakemake
set OPTS=-f -N -b %root% -c %root%\ipsuiteconfig.vc

set ALL_IPSUITE_INCLUDES=-I%root%/Network/IPv4 -I%root%/Network/IPv4d -I%root%/Network/AutoRouting -I%root%/Transport/TCP -I%root%/Transport/UDP -I%root%/NetworkInterfaces -I%root%/NetworkInterfaces/_802 -I%root%/NetworkInterfaces/ARP -I%root%/NetworkInterfaces/Ethernet -I%root%/NetworkInterfaces/PPP -I%root%/Applications/Generic -I%root%/Applications/Ethernet -I%root%/Applications/TCPApp -I%root%/Applications/UDPApp -I%root%/Applications/PingApp -I%root%/Base -I%root%/Util -I%root%/Nodes/IPSuite
set ALL_MPLS_IPSUITE_INCLUDES=%ALL_IPSUITE_INCLUDES% -I%root%/Network/MPLS -I%root%/Network/LDP -I%root%/Network/RSVP_TE -I%root%/Nodes/MPLS

:set ALL_MODEL_OPTS=%OPTS% -w %ALL_MPLS_IPSUITE_INCLUDES%
set ALL_MODEL_OPTS=%OPTS% -n

: #--------------------------------------

%MAKEMAKE% %OPTS% -n -r

cd %root%\Applications && %MAKEMAKE% %OPTS% -n -r
cd %root%\Examples && %MAKEMAKE% %OPTS% -n -r
cd %root%\Tests && %MAKEMAKE% %OPTS% -n -r
cd %root%\Network && %MAKEMAKE% %OPTS% -n -r
cd %root%\NetworkInterfaces && %MAKEMAKE% %OPTS% -n -r
cd %root%\Nodes && %MAKEMAKE% %OPTS% -n -r
cd %root%\PHY && %MAKEMAKE% %OPTS% -n -r
cd %root%\Transport && %MAKEMAKE% %OPTS% -n -r
cd %root%\Base && %MAKEMAKE% %OPTS% -n -r
cd %root%\Util && %MAKEMAKE% %OPTS% -n -r

cd %root%\Applications\Generic && %MAKEMAKE% %OPTS% -n -r -I..\..\Network\IPv4 -I..\..\Base -I..\..\Util
cd %root%\Applications\Ethernet && %MAKEMAKE% %OPTS% -n -r -I..\..\NetworkInterfaces\Ethernet -I..\..\NetworkInterfaces\_802 -I..\..\Base -I..\..\Util
cd %root%\Applications\PingApp && %MAKEMAKE% %OPTS% -n -r -I..\..\Network\IPv4 -I..\..\Base -I..\..\Util
cd %root%\Applications\TCPApp && %MAKEMAKE% %OPTS% -n -r -I..\..\Network\IPv4 -I..\..\Unsupported\TcpModule -I..\..\Transport\TCP -I..\..\Base -I..\..\Util
cd %root%\Applications\UDPApp && %MAKEMAKE% %OPTS% -n -r -I..\..\Network\IPv4 -I..\..\Unsupported\TcpModule -I..\..\Transport\UDP -I..\..\Base -I..\..\Util

cd %root%\Network\IPv4 && %MAKEMAKE% %OPTS% -n -r -I..\..\Base -I..\..\Util
cd %root%\Network\IPv4d && %MAKEMAKE% %OPTS% -n -r -I..\IPv4 -I..\..\Base -I..\..\Util
cd %root%\Network\AutoRouting && %MAKEMAKE% %OPTS% -n -r -I..\IPv4  -I..\..\Base -I..\..\Util
cd %root%\Network\MPLS && %MAKEMAKE% %OPTS% -n -r -I..\IPv4 -I..\IPv4d -I..\..\Unsupported\TcpModule -I..\..\Base -I..\..\Util
cd %root%\Network\LDP && %MAKEMAKE% %OPTS% -n -r -I..\IPv4 -I..\IPv4d -I..\..\Unsupported\TcpModule -I..\..\Transport\UDP -I..\MPLS -I..\..\Base -I..\..\Util
cd %root%\Network\RSVP_TE && %MAKEMAKE% %OPTS% -n -r -I..\IPv4 -I..\IPv4d -I..\..\Unsupported\TcpModule -I..\MPLS -I..\..\Base -I..\..\Util

cd %root%\NetworkInterfaces\PPP && %MAKEMAKE% %OPTS% -n -r -I..\..\Base -I..\..\Util -I..\..\Network\IPv4
cd %root%\NetworkInterfaces\_802 && %MAKEMAKE% %OPTS% -n -r -I..\..\Base -I..\..\Util -I..\..\Network\IPv4
cd %root%\NetworkInterfaces\Ethernet && %MAKEMAKE% %OPTS% -n -r -I..\..\Base -I..\..\Util -I..\_802 -I..\..\Network\IPv4
cd %root%\NetworkInterfaces\ARP && %MAKEMAKE% %OPTS% -n -r -I..\..\Base -I..\..\Util -I..\..\Network\IPv4 -I..\_802 -I..\Ethernet

cd %root%\Nodes\IPSuite && %MAKEMAKE% %OPTS% -n -r -I..\..\Network\IPv4 -I..\..\Network\IPv4d -I..\..\Network\IPv4\QoS -I..\..\Unsupported\TcpModule -I..\..\Transport\UDP -I..\..\NetworkInterfaces\PPP -I..\..\NetworkInterfaces  -I..\..\Applications\Generic -I..\..\Applications\TCPApp -I..\..\Applications\UDPApp -I..\..\Applications\PingApp -I..\..\Base -I..\..\Util
cd %root%\Nodes\MPLS && %MAKEMAKE% %OPTS% -n -r -I..\..\Network\IPv4 -I..\..\Network\IPv4d -I..\..\Network\IPv4\QoS -I..\..\Network\MPLS -I..\..\Network\LDP -I..\..\Network\RSVP_TE -I..\..\Unsupported\TcpModule -I..\..\Transport\UDP -I..\..\NetworkInterfaces\PPP -I..\..\NetworkInterfaces  -I..\..\Applications\Generic -I..\..\Applications\TCPApp -I..\..\Applications\PingApp -I..\IPSuite -I..\..\Base -I..\..\Util

cd %root%\Transport\UDP && %MAKEMAKE% %OPTS% -n -r -I..\..\Network\IPv4 -I..\..\Base -I..\..\Util
cd %root%\Transport\RTP && %MAKEMAKE% %OPTS% -n -r -I..\..\Network\IPv4 -I..\..\Base -I..\..\Util
cd %root%\Transport\TCP && %MAKEMAKE% %OPTS% -n -I..\..\Network\IPv4 -I..\..\Base -I..\..\Util

cd %root%\Unsupported\TcpModule && %MAKEMAKE% %OPTS% -n -r -I..\..\Network\IPv4 -I..\..\Applications\TCPApp -I..\..\Base -I..\..\Util
cd %root%\Unsupported\Socket && %MAKEMAKE% %OPTS% -n -r -I..\TcpModule -I..\UDP -I..\..\Network\IPv4 -I..\..\Base -I..\..\Util

cd %root%\Examples\bin && %MAKEMAKE% %OPTS% -w -o IPSuite %ALL_MPLS_IPSUITE_INCLUDES%

cd %root%\Examples\Ethernet && %MAKEMAKE% %OPTS% -n -r
cd %root%\Examples\IPSuite && %MAKEMAKE% %OPTS% -n -r
cd %root%\Examples\MPLS && %MAKEMAKE% %OPTS% -n -r

cd %root%\Examples\Ethernet\ARPTest && %MAKEMAKE% %ALL_MODEL_OPTS%
cd %root%\Examples\Ethernet\LANs && %MAKEMAKE% %ALL_MODEL_OPTS%

cd %root%\Examples\IPSuite\NClients && %MAKEMAKE% %ALL_MODEL_OPTS%
cd %root%\Examples\IPSuite\FlatNet && %MAKEMAKE% %ALL_MODEL_OPTS%
cd %root%\Examples\IPSuite\KIDSNw1 && %MAKEMAKE% %ALL_MODEL_OPTS%
cd %root%\Examples\IPSuite\McNetwork2 && %MAKEMAKE% %ALL_MODEL_OPTS%
cd %root%\Examples\IPSuite\PerformAnalysis && %MAKEMAKE% %ALL_MODEL_OPTS%
cd %root%\Examples\IPSuite\TCPClientServerNet && %MAKEMAKE% %ALL_MODEL_OPTS%

cd %root%\Examples\MPLS\Tester && %MAKEMAKE% %OPTS% -w -o MPLSTester %ALL_MPLS_IPSUITE_INCLUDES%
cd %root%\Examples\MPLS\demo2 && %MAKEMAKE% %ALL_MODEL_OPTS%
cd %root%\Examples\MPLS\ldp-mpls1 && %MAKEMAKE% %ALL_MODEL_OPTS%
cd %root%\Examples\MPLS\TestTE1 && %MAKEMAKE% %ALL_MODEL_OPTS% -I..\Tester
cd %root%\Examples\MPLS\TestTE2 && %MAKEMAKE% %ALL_MODEL_OPTS% -I..\Tester
cd %root%\Examples\MPLS\TestTE3 && %MAKEMAKE% %ALL_MODEL_OPTS% -I..\Tester
cd %root%\Examples\MPLS\TestTE4 && %MAKEMAKE% %ALL_MODEL_OPTS% -I..\Tester
cd %root%\Examples\MPLS\TestTE4Old && %MAKEMAKE% %ALL_MODEL_OPTS% -I..\Tester
cd %root%\Examples\MPLS\TestTE5 && %MAKEMAKE% %ALL_MODEL_OPTS% -I..\Tester
cd %root%\Examples\MPLS\TestTE6 && %MAKEMAKE% %ALL_MODEL_OPTS% -I..\Tester

cd %root%\Tests\MPLS && %MAKEMAKE% %OPTS% -n -r
cd %root%\Tests\MPLS\LDP1 && %MAKEMAKE% %OPTS% -w %ALL_MPLS_IPSUITE_INCLUDES%
cd %root%\Tests\NewTCP && %MAKEMAKE% %OPTS% -w %ALL_MPLS_IPSUITE_INCLUDES%

: #--------------------------------------

cd %root%
dir /s/b *.ned > nedfiles.lst
perl -i.bak -pe "s/.*[^d]\n$//;s|\\|/|g;s|.*?IPSuite.*?/||" nedfiles.lst
perl -i.bak -pe "s|^Examples/.*||" nedfiles.lst
perl -i.bak -pe "s|^Unsupported/.*||" nedfiles.lst
perl -i.bak -pe "s|^Tests/.*||" nedfiles.lst
