set root=D:\home\IPSuite-local
set MAKEMAKE=cmd /c d:\home\omnetpp\bin\opp_nmakemake
set OPTS=-f -N
:set OPTS=-f

set ALL_IPSUITE_INCLUDES=-I$(ROOT)/Network/IPv4/Core -I$(ROOT)/Network/IPv4/QoS -I$(ROOT)/Transport/TCP -I$(ROOT)/Transport/UDP -I$(ROOT)/Transport/Socket -I$(ROOT)/NetworkInterfaces -I$(ROOT)/NetworkInterfaces/PPP-old -I$(ROOT)/NetworkInterfaces/Queues -I$(ROOT)/Applications/Generic -I$(ROOT)/Applications/TCPApp -I$(ROOT)/Applications/UDPApp -I$(ROOT)/Applications/PingApp -I$(ROOT)/Base -I$(ROOT)/Util -I$(ROOT)/Nodes/IPSuite
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

cd %root%\Applications\Generic && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Base
cd %root%\Applications\PingApp && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Base
cd %root%\Applications\TCPApp && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Transport\TCP -I..\..\Base
cd %root%\Applications\UDPApp && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Transport\TCP -I..\..\Transport\UDP -I..\..\Base

cd %root%\Examples\IPSuite && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc
cd %root%\Examples\MPLS && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc

cd %root%\Examples\IPSuite\KIDSNw1 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\McNetwork2 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\OldNetwork && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\PerformAnalysis && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\TCPClientServerNet && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\TCPFlavours && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\TCPTester && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\TCPUDPDirectNet && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\UDPSockets && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%

cd %root%\Examples\MPLS\Tester && %MAKEMAKE% %OPTS% -n -r -c ..\..\..\ipsuiteconfig.vc %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\demo2 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\ldp-mpls1 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\samples && %MAKEMAKE% %OPTS% -n -r -c ..\..\..\ipsuiteconfig.vc
cd %root%\Examples\MPLS\TestTE1 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\TestTE2 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\TestTE3 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\TestTE4 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\TestTE4Old && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\TestTE5 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\TestTE6 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES%

cd %root%\Tests\MPLS && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc
cd %root%\Tests\MPLS\LDP1 && %MAKEMAKE% %OPTS% -w -c ..\..\..\ipsuiteconfig.vc %ALL_MPLS_INCLUDES%

cd %root%\Network\IPv4 && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc
cd %root%\Network\MPLS && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\IPv4\Core -I..\..\Transport\TCP -I..\..\Base
cd %root%\Network\LDP && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\IPv4\Core -I..\..\Transport\TCP -I..\..\Transport\UDP -I..\MPLS -I..\..\Base
cd %root%\Network\RSVP_TE && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\IPv4\Core -I..\..\Transport\TCP -I..\MPLS -I..\..\Base
cd %root%\Network\IPv4\Core && %MAKEMAKE% %OPTS% -n -r -c ..\..\..\ipsuiteconfig.vc -I..\..\..\NetworkInterfaces -I..\..\..\NetworkInterfaces\Queues -I..\..\..\Base
cd %root%\Network\IPv4\QoS && %MAKEMAKE% %OPTS% -n -r -c ..\..\..\ipsuiteconfig.vc -I..\Core -I..\..\..\Transport\TCP -I..\..\..\Base

cd %root%\NetworkInterfaces\PPP-old && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Base -I..\..\Network\IPv4\Core
cd %root%\NetworkInterfaces\Queues && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Base -I..\..\Network\IPv4\Core -I..\..\Network\IPv4\QoS

cd %root%\Nodes\IPSuite && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Network\IPv4\QoS -I..\..\Transport\TCP -I..\..\Transport\UDP -I..\..\NetworkInterfaces\PPP-old -I..\..\NetworkInterfaces -I..\..\NetworkInterfaces\Queues  -I..\..\Applications\Generic -I..\..\Applications\TCPApp -I..\..\Applications\UDPApp -I..\..\Applications\PingApp -I..\..\Base
cd %root%\Nodes\MPLS && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Network\IPv4\QoS -I..\..\Network\MPLS -I..\..\Network\LDP -I..\..\Network\RSVP_TE -I..\..\Transport\TCP -I..\..\Transport\UDP -I..\..\NetworkInterfaces\PPP-old -I..\..\NetworkInterfaces -I..\..\NetworkInterfaces\Queues  -I..\..\Applications\Generic -I..\..\Applications\TCPApp -I..\..\Applications\PingApp -I..\IPSuite -I..\..\Base

cd %root%\Transport\Socket && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\TCP -I..\..\Network\IPv4\Core -I..\..\Base
cd %root%\Transport\TCP && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Applications\TCPApp -I..\..\Base
cd %root%\Transport\UDP && %MAKEMAKE% %OPTS% -n -r -c ..\..\ipsuiteconfig.vc -I..\TCP -I..\..\Network\IPv4\Core -I..\..\Base

: #--------------------------------------

cd %root%
dir /s/b *.ned > nedfiles.lst
perl -i.bak -pe "s/.*[^d]\n$//;s|\\|/|g;s|.*?IPSuite-local/||" nedfiles.lst
perl -i.bak -pe "s|^Examples/.*||" nedfiles.lst
