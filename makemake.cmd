@echo off
set ROOT=C:\home\ipsuite-rearranged
set ICONV=C:/home/tools/iconv-1.8.win32
set LIBXML=C:/home/tools/libxml2-2.5.4.win32

cd %ROOT%
cd && call opp_nmakemake -f -n -c ipsuiteconfig.vc Network Applications Transport NetworkInterfaces PHY Util Nodes

cd %ROOT%\Applications
cd && call opp_nmakemake -f -n -c ..\ipsuiteconfig.vc

cd %ROOT%\Examples
cd && call opp_nmakemake -f -n -c ..\ipsuiteconfig.vc

cd %ROOT%\Network
cd && call opp_nmakemake -f -n -c ..\ipsuiteconfig.vc

cd %ROOT%\NetworkInterfaces
cd && call opp_nmakemake -f -n -c ..\ipsuiteconfig.vc
 
cd %ROOT%\Nodes
cd && call opp_nmakemake -f -n -c ..\ipsuiteconfig.vc

cd %ROOT%\PHY
cd && call opp_nmakemake -f -n -c ..\ipsuiteconfig.vc

cd %ROOT%\Transport
cd && call opp_nmakemake -f -n -c ..\ipsuiteconfig.vc

cd %ROOT%\Util
cd && call opp_nmakemake -f -n -c ..\ipsuiteconfig.vc

cd %ROOT%\Applications\Generic
cd && call opp_nmakemake -f -n -c ..\..\ipsuiteconfig.vc -I../../Network/IPv4/Core -I../../Util

cd %ROOT%\Applications\PingApp
cd && call opp_nmakemake -f -n -c ..\..\ipsuiteconfig.vc -I../../Network/IPv4/Core -I../../Util

cd %ROOT%\Applications\TCPApp
cd && call opp_nmakemake -f -n -c ..\..\ipsuiteconfig.vc -I../../Network/IPv4/Core -I../../Transport/TCPv4 -I../../Util

cd %ROOT%\Applications\TCPExample
cd && call opp_nmakemake -f -n -c ..\..\ipsuiteconfig.vc -I../../Network/IPv4/Core -I../../Transport/TCPv4 -I../../Network/MPLS -I../../Network/LDP -I../../Util

cd %ROOT%\Examples\IPSuite
cd && call opp_nmakemake -f -n -c ..\..\ipsuiteconfig.vc

cd %ROOT%\Examples\IPSuite\KIDSNw1
cd && call opp_nmakemake -f -n -c ..\..\..\ipsuiteconfig.vc -I../../../Nodes/IPSuite -I../../../Network/IPv4/Core -I../../../Network/IPv4/QoS -I../../../Transport/TCPv4 -I../../../Transport/UDPv4 -I../../../NetworkInterfaces/PPP-old -I../../../NetworkInterfaces -I../../../NetworkInterfaces/Queues  -I../../../Applications/Generic -I../../../Applications/PingApp -I../../../Util

cd %ROOT%\Examples\IPSuite\McNetwork2
:cd && call opp_nmakemake -f -c ..\..\..\ipsuiteconfig.vc -I../../../Nodes/IPSuite -I../../../Network/IPv4/Core -I../../../Network/IPv4/QoS -I../../../Transport/TCPv4 -I../../../Transport/UDPv4 -I../../../NetworkInterfaces/PPP-old -I../../../NetworkInterfaces -I../../../NetworkInterfaces/Queues  -I../../../Applications/Generic -I../../../Applications/PingApp -I../../../Util
cd && call opp_nmakemake -f -c ..\..\..\ipsuiteconfig.vc -I../../../Nodes/IPSuite ../../../Nodes/IPSuite -I../../../Network/IPv4/Core ../../../Network/IPv4/Core -I../../../Network/IPv4/QoS ../../../Network/IPv4/QoS -I../../../Transport/TCPv4 ../../../Transport/TCPv4 -I../../../Transport/UDPv4 ../../../Transport/UDPv4 -I../../../NetworkInterfaces/PPP-old ../../../NetworkInterfaces/PPP-old -I../../../NetworkInterfaces ../../../NetworkInterfaces -I../../../NetworkInterfaces/Queues ../../../NetworkInterfaces/Queues  -I../../../Applications/Generic ../../../Applications/Generic -I../../../Applications/PingApp ../../../Applications/PingApp -I../../../Util ../../../Util

cd %ROOT%\Examples\IPSuite\OldNetwork
cd && call opp_nmakemake -f -n -c ..\..\..\ipsuiteconfig.vc -I../../../Nodes/IPSuite -I../../../Network/IPv4/Core -I../../../Network/IPv4/QoS -I../../../Transport/TCPv4 -I../../../Transport/UDPv4 -I../../../NetworkInterfaces/PPP-old -I../../../NetworkInterfaces -I../../../NetworkInterfaces/Queues  -I../../../Applications/Generic -I../../../Applications/PingApp -I../../../Util

cd %ROOT%\Examples\IPSuite\PerformAnalysis
:cd && call opp_nmakemake -f -n -c ..\..\..\ipsuiteconfig.vc -I../../../Nodes/IPSuite -I../../../Network/IPv4/Core -I../../../Network/IPv4/QoS -I../../../Transport/TCPv4 -I../../../Transport/UDPv4 -I../../../NetworkInterfaces/PPP-old -I../../../NetworkInterfaces -I../../../NetworkInterfaces/Queues  -I../../../Applications/Generic -I../../../Applications/PingApp -I../../../Util
cd && call opp_nmakemake -f -c ..\..\..\ipsuiteconfig.vc -I../../../Nodes/IPSuite ../../../Nodes/IPSuite -I../../../Network/IPv4/Core ../../../Network/IPv4/Core -I../../../Network/IPv4/QoS ../../../Network/IPv4/QoS -I../../../Transport/TCPv4 ../../../Transport/TCPv4 -I../../../Transport/UDPv4 ../../../Transport/UDPv4 -I../../../NetworkInterfaces/PPP-old ../../../NetworkInterfaces/PPP-old -I../../../NetworkInterfaces ../../../NetworkInterfaces -I../../../NetworkInterfaces/Queues ../../../NetworkInterfaces/Queues  -I../../../Applications/Generic ../../../Applications/Generic -I../../../Applications/PingApp ../../../Applications/PingApp -I../../../Util ../../../Util

cd %ROOT%\Examples\IPSuite\SocketTester
cd && call opp_nmakemake -f -n -c ..\..\..\ipsuiteconfig.vc -I../../../Nodes/IPSuite -I../../../Network/IPv4/Core -I../../../Network/IPv4/QoS -I../../../Transport/TCPv4 -I../../../Transport/UDPv4 -I../../../NetworkInterfaces/PPP-old -I../../../NetworkInterfaces -I../../../NetworkInterfaces/Queues  -I../../../Applications/Generic -I../../../Applications/PingApp -I../../../Util

cd %ROOT%\Examples\IPSuite\TCPClientServerNet
cd && call opp_nmakemake -f -n -c ..\..\..\ipsuiteconfig.vc -I../../../Nodes/IPSuite -I../../../Network/IPv4/Core -I../../../Network/IPv4/QoS -I../../../Transport/TCPv4 -I../../../Transport/UDPv4 -I../../../NetworkInterfaces/PPP-old -I../../../NetworkInterfaces -I../../../NetworkInterfaces/Queues  -I../../../Applications/Generic -I../../../Applications/PingApp -I../../../Util

cd %ROOT%\Examples\IPSuite\TCPFlavours
cd && call opp_nmakemake -f -n -c ..\..\..\ipsuiteconfig.vc -I../../../Nodes/IPSuite -I../../../Network/IPv4/Core -I../../../Network/IPv4/QoS -I../../../Transport/TCPv4 -I../../../Transport/UDPv4 -I../../../NetworkInterfaces/PPP-old -I../../../NetworkInterfaces -I../../../NetworkInterfaces/Queues  -I../../../Applications/Generic -I../../../Applications/PingApp -I../../../Util

cd %ROOT%\Examples\IPSuite\TCPTester
cd && call opp_nmakemake -f -n -c ..\..\..\ipsuiteconfig.vc -I../../../Nodes/IPSuite -I../../../Network/IPv4/Core -I../../../Network/IPv4/QoS -I../../../Transport/TCPv4 -I../../../Transport/UDPv4 -I../../../NetworkInterfaces/PPP-old -I../../../NetworkInterfaces -I../../../NetworkInterfaces/Queues  -I../../../Applications/Generic -I../../../Applications/PingApp -I../../../Util

cd %ROOT%\Examples\IPSuite\TCPUDPDirectNet
cd && call opp_nmakemake -f -n -c ..\..\..\ipsuiteconfig.vc -I../../../Nodes/IPSuite -I../../../Network/IPv4/Core -I../../../Network/IPv4/QoS -I../../../Transport/TCPv4 -I../../../Transport/UDPv4 -I../../../NetworkInterfaces/PPP-old -I../../../NetworkInterfaces -I../../../NetworkInterfaces/Queues  -I../../../Applications/Generic -I../../../Applications/PingApp -I../../../Util

cd %ROOT%\Examples\IPSuite\UDPSockets
cd && call opp_nmakemake -f -n -c ..\..\..\ipsuiteconfig.vc -I../../../Nodes/IPSuite -I../../../Network/IPv4/Core -I../../../Network/IPv4/QoS -I../../../Transport/TCPv4 -I../../../Transport/UDPv4 -I../../../Transport/Socketv4 -I../../../NetworkInterfaces/PPP-old -I../../../NetworkInterfaces -I../../../NetworkInterfaces/Queues  -I../../../Applications/Generic -I../../../Applications/PingApp -I../../../Util

cd %ROOT%\Network\IPv4
cd && call opp_nmakemake -f -n -c ..\..\ipsuiteconfig.vc 

cd %ROOT%\Network\MPLS
cd && call opp_nmakemake -f -n -c ..\..\ipsuiteconfig.vc -I../IPv4/Core -I../../Transport/TCPv4 -I../../Util

cd %ROOT%\Network\LDP
cd && call opp_nmakemake -f -n -c ..\..\ipsuiteconfig.vc -I../IPv4/Core -I../../Transport/TCPv4 -I../../Transport/UDPv4 -I../MPLS -I../../Util

cd %ROOT%\Network\RSVP_TE
cd && call opp_nmakemake -f -n -c ..\..\ipsuiteconfig.vc -I../IPv4/Core -I../../Transport/TCPv4 -I../MPLS -I../../Util -I%LIBXML%/include -I%ICONV%/include

cd %ROOT%\Network\IPv4\Core
cd && call opp_nmakemake -f -n -c ..\..\..\ipsuiteconfig.vc -I../QoSStub -I../../../NetworkInterfaces -I../../../NetworkInterfaces/Queues -I../../../Util

cd %ROOT%\Network\IPv4\QoS
cd && call opp_nmakemake -f -n -c ..\..\..\ipsuiteconfig.vc -I../QoSStub -I../Core -I../../../Transport/TCPv4 -I../../../Util

cd %ROOT%\Network\IPv4\QoSStub
cd && call opp_nmakemake -f -n -c ..\..\..\ipsuiteconfig.vc -I../Core -I../../../Util

cd %ROOT%\NetworkInterfaces\PPP-old
cd && call opp_nmakemake -f -n -c ..\..\ipsuiteconfig.vc -I../../Util -I../../Network/IPv4/Core

cd %ROOT%\NetworkInterfaces\Queues
cd && call opp_nmakemake -f -n -c ..\..\ipsuiteconfig.vc -I../../Util -I../../Network/IPv4/Core -I../../Network/IPv4/QoS

cd %ROOT%\Nodes\IPSuite
cd && call opp_nmakemake -f -n -c ..\..\ipsuiteconfig.vc -I../../Network/IPv4/Core -I../../Network/IPv4/QoS -I../../Transport/TCPv4 -I../../Transport/UDPv4 -I../../NetworkInterfaces/PPP-old -I../../NetworkInterfaces -I../../NetworkInterfaces/Queues  -I../../Applications/Generic -I../../Applications/PingApp -I../../Util

cd %ROOT%\Nodes\MPLS
cd && call opp_nmakemake -f -n -c ..\..\ipsuiteconfig.vc -I../../Network/IPv4/Core -I../../Network/IPv4/QoS -I../../Network/MPLS -I../../Network/LDP -I../../Network/RSVP_TE -I../../Transport/TCPv4 -I../../Transport/UDPv4 -I../../NetworkInterfaces/PPP-old -I../../NetworkInterfaces -I../../NetworkInterfaces/Queues  -I../../Applications/Generic -I../../Applications/PingApp -I../../Util

cd %ROOT%\Transport\Socketv4
cd && call opp_nmakemake -f -n -c ..\..\ipsuiteconfig.vc -I../TCPv4 -I../../Network/IPv4/Core -I../../Util

cd %ROOT%\Transport\TCPv4
cd && call opp_nmakemake -f -n -c ..\..\ipsuiteconfig.vc -I../../Network/IPv4/Core -I../../Util

cd %ROOT%\Transport\UDPv4
cd && call opp_nmakemake -f -n -c ..\..\ipsuiteconfig.vc -I../TCPv4 -I../../Network/IPv4/Core -I../../Util

cd %ROOT%

