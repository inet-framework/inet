set root=D:\home\IPSuite-pcvs
set MAKEMAKE=cmd /c d:\home\omnetpp\bin\opp_nmakemake
set OPTS=-f -N
:set OPTS=-f

set ALL_IPSUITE_INCLUDES=-I$(ROOT)/Network/IPv4/Core -I$(ROOT)/Network/IPv4/QoS -I$(ROOT)/Transport/TCP -I$(ROOT)/Transport/NewTCP -I$(ROOT)/Transport/UDP -I$(ROOT)/Transport/Socket -I$(ROOT)/NetworkInterfaces -I$(ROOT)/NetworkInterfaces/_802 -I$(ROOT)/NetworkInterfaces/ARP -I$(ROOT)/NetworkInterfaces/Ethernet -I$(ROOT)/NetworkInterfaces/PPP -I$(ROOT)/NetworkInterfaces/Queues -I$(ROOT)/Applications/Generic -I$(ROOT)/Applications/TCPApp -I$(ROOT)/Applications/UDPApp -I$(ROOT)/Applications/PingApp -I$(ROOT)/Base -I$(ROOT)/Util -I$(ROOT)/Nodes/IPSuite
set ALL_MPLS_INCLUDES=%ALL_IPSUITE_INCLUDES% -I$(ROOT)/Network/MPLS -I$(ROOT)/Network/LDP -I$(ROOT)/Network/RSVP_TE -I$(ROOT)/Nodes/MPLS

: #--------------------------------------

%MAKEMAKE% %OPTS% -n -r -c ipsuiteconfig.vc

cd %root%\Applications && %MAKEMAKE% %OPTS% -n -r -c ..\ipsuiteconfig.vc
cd %root%\Examples && %MAKEMAKE% %OPTS% -n -r -c ..\ipsuiteconfig.vc
cd %root%\Tests && %MAKEMAKE% %OPTS% -n -r -c ..\ipsuiteconfig.vc
cd %root%\Network && %MAKEMAKE% %OPTS% -n -r -c ..\ipsuiteconfig.vc
cd %root%\NetworkInterfaces && %MAKEMAKE% %OPTS% -n -r -c ..\ipsuiteconfig.vc -IQueues
cd %root%\Nodes && %MAKEMAKE% %OPTS% -n -r -c ..\ipsuiteconfig.vc
cd %root%\PHY && %MAKEMAKE% %OPTS% -n -r -c ..\ipsuiteconfig.vc
cd %root%\Transport && %MAKEMAKE% %OPTS% -n -r -c ..\ipsuiteconfig.vc
cd %root%\Base && %MAKEMAKE% %OPTS% -n -r -c ..\ipsuiteconfig.vc
cd %root%\Util && %MAKEMAKE% %OPTS% -n -r -c ..\ipsuiteconfig.vc

cd %root%\Applications\Generic && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Base -I..\..\Util
cd %root%\Applications\PingApp && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Base -I..\..\Util
cd %root%\Applications\TCPApp && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Transport\TCP -I..\..\Base -I..\..\Util
cd %root%\Applications\UDPApp && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Transport\TCP -I..\..\Transport\UDP -I..\..\Base -I..\..\Util

cd %root%\Examples\IPSuite && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc
cd %root%\Examples\MPLS && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc

cd %root%\Examples\Ethernet && %MAKEMAKE% %OPTS% -w -c ..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%

cd %root%\Examples\IPSuite\KIDSNw1 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\McNetwork2 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\PerformAnalysis && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\TCPClientServerNet && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\TCPTester && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\TCPUDPDirectNet && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\UDPSockets && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%

cd %root%\Examples\MPLS\Tester && %MAKEMAKE% %OPTS% -n -r -c ..\..\..\ipsuiteconfig.vc %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\demo2 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\ldp-mpls1 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\TestTE1 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\TestTE2 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\TestTE3 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\TestTE4 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\TestTE4Old && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\TestTE5 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\TestTE6 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES%

cd %root%\Tests\MPLS && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc
cd %root%\Tests\MPLS\LDP1 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_MPLS_INCLUDES%
cd %root%\Tests\NewTCP && %MAKEMAKE% %OPTS% -w -c ..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%

cd %root%\Network\IPv4 && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Base -I..\..\Util -ICore
cd %root%\Network\MPLS && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\IPv4\Core -I..\..\Transport\TCP -I..\..\Base -I..\..\Util
cd %root%\Network\LDP && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\IPv4\Core -I..\..\Transport\TCP -I..\..\Transport\UDP -I..\MPLS -I..\..\Base -I..\..\Util
cd %root%\Network\RSVP_TE && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\IPv4\Core -I..\..\Transport\TCP -I..\MPLS -I..\..\Base -I..\..\Util
cd %root%\Network\IPv4\Core && %MAKEMAKE% %OPTS% -n -r -c ..\..\..\ipsuiteconfig.vc -I..\..\..\NetworkInterfaces -I..\..\..\NetworkInterfaces\Queues -I..\..\..\Base -I..\..\..\Util
cd %root%\Network\IPv4\QoS && %MAKEMAKE% %OPTS% -n -r -c ..\..\..\ipsuiteconfig.vc -I..\Core -I..\..\..\Transport\TCP -I..\..\..\Base -I..\..\..\Util

cd %root%\NetworkInterfaces\PPP && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Base -I..\..\Util
cd %root%\NetworkInterfaces\_802 && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Base -I..\..\Util -I..\..\Network\IPv4\Core
cd %root%\NetworkInterfaces\Ethernet && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Base -I..\..\Util -I..\_802
cd %root%\NetworkInterfaces\ARP && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Base -I..\..\Util -I..\..\Network\IPv4\Core -I..\_802 -I..\Ethernet
cd %root%\NetworkInterfaces\Queues && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Base -I..\..\Util -I..\..\Network\IPv4\Core -I..\..\Network\IPv4\QoS

cd %root%\Nodes\IPSuite && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Network\IPv4\QoS -I..\..\Transport\TCP -I..\..\Transport\UDP -I..\..\NetworkInterfaces\PPP -I..\..\NetworkInterfaces -I..\..\NetworkInterfaces\Queues  -I..\..\Applications\Generic -I..\..\Applications\TCPApp -I..\..\Applications\UDPApp -I..\..\Applications\PingApp -I..\..\Base -I..\..\Util
cd %root%\Nodes\MPLS && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Network\IPv4\QoS -I..\..\Network\MPLS -I..\..\Network\LDP -I..\..\Network\RSVP_TE -I..\..\Transport\TCP -I..\..\Transport\UDP -I..\..\NetworkInterfaces\PPP -I..\..\NetworkInterfaces -I..\..\NetworkInterfaces\Queues  -I..\..\Applications\Generic -I..\..\Applications\TCPApp -I..\..\Applications\PingApp -I..\IPSuite -I..\..\Base -I..\..\Util

cd %root%\Transport\Socket && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\TCP -I..\UDP -I..\..\Network\IPv4\Core -I..\..\Base -I..\..\Util
cd %root%\Transport\TCP && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Applications\TCPApp -I..\..\Base -I..\..\Util
cd %root%\Transport\UDP && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\TCP -I..\..\Network\IPv4\Core -I..\..\Base -I..\..\Util
cd %root%\Transport\RTP && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Base -I..\..\Util
cd %root%\Transport\NewTCP && %MAKEMAKE% %OPTS% -n -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Base -I..\..\Util

: #--------------------------------------

cd %root%
dir /s/b *.ned > nedfiles.lst
perl -i.bak -pe "s/.*[^d]\n$//;s|\\|/|g;s|.*?IPSuite.*?/||" nedfiles.lst
perl -i.bak -pe "s|^Examples/.*||" nedfiles.lst
