set root=C:\home\opp-rsvp-te\ipsuite-rearranged
:set MAKEMAKE=call %root%\opp_nmakemake
set MAKEMAKE=cmd /c %root%\opp_nmakemake

set ICONV=C:/home/tools/iconv-1.8.win32
set LIBXML=C:/home/tools/libxml2-2.5.4.win32

set LIBXML_INCLUDES=-I%LIBXML%/include -I%ICONV%/include
set LIBXML_LIBS=%LIBXML%/lib/libxml2.lib %ICONV%/lib/iconv.lib
set ALL_IPSUITE_INCLUDES=-I$(ROOT)/Network/IPv4/Core -I$(ROOT)/Network/IPv4/QoS -I$(ROOT)/Transport/TCPv4 -I$(ROOT)/Transport/UDPv4 -I$(ROOT)/Transport/Socketv4 -I$(ROOT)/NetworkInterfaces -I$(ROOT)/NetworkInterfaces/PPP-old -I$(ROOT)/NetworkInterfaces/Queues -I$(ROOT)/Applications/Generic -I$(ROOT)/Applications/TCPApp -I$(ROOT)/Applications/PingApp -I$(ROOT)/Util -I$(ROOT)/Nodes/IPSuite
set ALL_MPLS_INCLUDES=%ALL_IPSUITE_INCLUDES% -I$(ROOT)/Network/MPLS -I$(ROOT)/Network/LDP -I$(ROOT)/Network/RSVP_TE -I$(ROOT)/Nodes/MPLS

:#--------------------------------------

%MAKEMAKE% -f -n -r -c ipsuiteconfig.vc

cd %root%\Applications && %MAKEMAKE% -f -n -r -c ..\ipsuiteconfig.vc
cd %root%\Examples && %MAKEMAKE% -f -n -r -c ..\ipsuiteconfig.vc
cd %root%\Tests && %MAKEMAKE% -f -n -r -c ..\ipsuiteconfig.vc
cd %root%\Network && %MAKEMAKE% -f -n -r -c ..\ipsuiteconfig.vc
cd %root%\NetworkInterfaces && %MAKEMAKE% -f -n -r -c ..\ipsuiteconfig.vc
cd %root%\Nodes && %MAKEMAKE% -f -n -r -c ..\ipsuiteconfig.vc
cd %root%\PHY && %MAKEMAKE% -f -n -r -c ..\ipsuiteconfig.vc
cd %root%\Transport && %MAKEMAKE% -f -n -r -c ..\ipsuiteconfig.vc
cd %root%\Util && %MAKEMAKE% -f -n -r -c ..\ipsuiteconfig.vc

cd %root%\Applications\Generic && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Util
cd %root%\Applications\PingApp && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Util
cd %root%\Applications\TCPApp && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Transport\TCPv4 -I..\..\Util
cd %root%\Applications\TCPExample && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc -I..\TCPApp -I..\..\Network\IPv4\Core -I..\..\Transport\TCPv4 -I..\..\Network\MPLS -I..\..\Network\LDP -I..\..\Util

cd %root%\Examples\IPSuite && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc
cd %root%\Examples\MPLS && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc

cd %root%\Examples\IPSuite\KIDSNw1 && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\McNetwork2 && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\OldNetwork && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\PerformAnalysis && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\SocketTester && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\TCPClientServerNet && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\TCPFlavours && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\TCPTester && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\TCPUDPDirectNet && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%
cd %root%\Examples\IPSuite\UDPSockets && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc %ALL_IPSUITE_INCLUDES%

cd %root%\Examples\MPLS\Tester && %MAKEMAKE% -f -n -r -c ..\..\..\ipsuiteconfig.vc %ALL_MPLS_INCLUDES%
cd %root%\Examples\MPLS\demo2 && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc %ALL_MPLS_INCLUDES% %LIBXML_LIBS%
cd %root%\Examples\MPLS\ldp-mpls1 && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc %ALL_MPLS_INCLUDES% %LIBXML_LIBS%
cd %root%\Examples\MPLS\samples && %MAKEMAKE% -f -n -r -c ..\..\..\ipsuiteconfig.vc
cd %root%\Examples\MPLS\TestTE1 && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES% %LIBXML_LIBS%
cd %root%\Examples\MPLS\TestTE2 && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES% %LIBXML_LIBS%
cd %root%\Examples\MPLS\TestTE3 && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES% %LIBXML_LIBS%
cd %root%\Examples\MPLS\TestTE4 && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES% %LIBXML_LIBS%
cd %root%\Examples\MPLS\TestTE4Old && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES% %LIBXML_LIBS%
cd %root%\Examples\MPLS\TestTE5 && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES% %LIBXML_LIBS%
cd %root%\Examples\MPLS\TestTE6 && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc -I..\Tester %ALL_MPLS_INCLUDES% %LIBXML_LIBS%

cd %root%\Tests\MPLS && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc
cd %root%\Tests\MPLS\LDP1 && %MAKEMAKE% -f -w -c ..\..\..\ipsuiteconfig.vc %ALL_MPLS_INCLUDES% %LIBXML_LIBS%

cd %root%\Network\IPv4 && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc 
cd %root%\Network\MPLS && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc -I..\IPv4\Core -I..\..\Transport\TCPv4 -I..\..\Util
cd %root%\Network\LDP && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc -I..\IPv4\Core -I..\..\Transport\TCPv4 -I..\..\Transport\UDPv4 -I..\MPLS -I..\..\Util
cd %root%\Network\RSVP_TE && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc -I..\IPv4\Core -I..\..\Transport\TCPv4 -I..\MPLS -I..\..\Util %LIBXML_INCLUDES%
cd %root%\Network\IPv4\Core && %MAKEMAKE% -f -n -r -c ..\..\..\ipsuiteconfig.vc -I..\QoSStub -I..\..\..\NetworkInterfaces -I..\..\..\NetworkInterfaces\Queues -I..\..\..\Util
cd %root%\Network\IPv4\QoS && %MAKEMAKE% -f -n -r -c ..\..\..\ipsuiteconfig.vc -I..\QoSStub -I..\Core -I..\..\..\Transport\TCPv4 -I..\..\..\Util
cd %root%\Network\IPv4\QoSStub && %MAKEMAKE% -f -n -r -c ..\..\..\ipsuiteconfig.vc -I..\Core -I..\..\..\Util

cd %root%\NetworkInterfaces\PPP-old && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Util -I..\..\Network\IPv4\Core
cd %root%\NetworkInterfaces\Queues && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Util -I..\..\Network\IPv4\Core -I..\..\Network\IPv4\QoS

cd %root%\Nodes\IPSuite && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Network\IPv4\QoS -I..\..\Transport\TCPv4 -I..\..\Transport\UDPv4 -I..\..\NetworkInterfaces\PPP-old -I..\..\NetworkInterfaces -I..\..\NetworkInterfaces\Queues  -I..\..\Applications\Generic -I..\..\Applications\TCPApp -I..\..\Applications\PingApp -I..\..\Util
cd %root%\Nodes\MPLS && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Network\IPv4\QoS -I..\..\Network\MPLS -I..\..\Network\LDP -I..\..\Network\RSVP_TE -I..\..\Transport\TCPv4 -I..\..\Transport\UDPv4 -I..\..\NetworkInterfaces\PPP-old -I..\..\NetworkInterfaces -I..\..\NetworkInterfaces\Queues  -I..\..\Applications\Generic -I..\..\Applications\TCPApp -I..\..\Applications\PingApp -I..\IPSuite -I..\..\Util

cd %root%\Transport\Socketv4 && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc -I..\TCPv4 -I..\..\Network\IPv4\Core -I..\..\Util
cd %root%\Transport\TCPv4 && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc -I..\..\Network\IPv4\Core -I..\..\Applications\TCPApp -I..\..\Util
cd %root%\Transport\UDPv4 && %MAKEMAKE% -f -n -r -c ..\..\ipsuiteconfig.vc -I..\TCPv4 -I..\..\Network\IPv4\Core -I..\..\Util

