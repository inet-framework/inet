//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.nodes.inet;

import inet.transport.IUDP;
import inet.transport.ITCP;
import inet.transport.ISCTP;
import inet.applications.pingapp.PingApp;
import inet.applications.IUDPApp;
import inet.applications.ITCPApp;
import inet.applications.ISCTPApp;


//
// IPv4 host with SCTP, TCP, UDP layers and applications.
// IP forwarding is disabled by default (see IPForward).
// - Can be connected via ethernet interface to other nodes using
//   the ethg gate. By default full duplex connections are supported
//   only (twisted pair). Set **.eth[*].typename="EthernetInterface" for
//   a full/half duplex CSMA/CD implementation (coaxial cable)
// - By default contains no wireless cards, however it can be configured
//   by the numRadios parameter. Wirless card type is configured by the
//   **.wlan[*].typename parameter. see:  inet.linklayer.ieee80211 or other
//   modules implementing ~IWirelessNic
// - Also external interfaces can be configured for HW in the loop simulation
//   using the numExtInterfaces parameter and setting the type using
//   **.ext[*].typename in the INI file. see: ~ExtInerface and ~IExternalNic
// - PPP interface type can be set using **.ppp[*].typename see: ~PPPInterface
//   and ~PPPInterfaceNoQueue
// - If wireless card is present, node mobility can be set using **.mobilityType
//   see: inet.mobility and ~IMobility
//
module StandardHost extends NodeBase
{
    parameters:
        @display("i=device/pc2");
        int numTcpApps = default(0);  // no of TCP apps. Specify the app types in INI file with tcpApp[0..1].typename="TCPEchoApp" syntax
        int numUdpApps = default(0);  // no of UDP apps. Specify the app types in INI file with udpApp[0..1].typename="UDPVideoStreamCli" syntax
        int numSctpApps = default(0); // no of SCTP apps. Specify the app types in INI file with sctpApp[0..1].typename="SCTPServer" syntax
        string tcpType = default(firstAvailable("TCP", "TCP_lwip", "TCP_NSC", "TCP_None"));  // tcp implementation (e.g. ~TCP, ~TCP_lwip, ~TCP_NSC) or ~TCPSpoof
        string udpType = default(firstAvailable("UDP","UDP_None"));
        string sctpType = default(firstAvailable("SCTP","SCTP_None"));
        IPForward = default(false);  // disable routing by default
        networkLayer.proxyARP = default(false);
    submodules:
        tcpApp[numTcpApps]: <> like ITCPApp {
            parameters:
                @display("p=186,54");
        }
        tcp: <tcpType> like ITCP if numTcpApps>0 {
            parameters:
                @display("p=186,141");
        }
        udpApp[numUdpApps]: <> like IUDPApp {
            parameters:
                @display("p=336,54");
        }
        udp: <udpType> like IUDP if numUdpApps>0 {
            parameters:
                @display("p=336,141");
        }
        sctpApp[numSctpApps]: <> like ISCTPApp {
            parameters:
                @display("p=501,54");
        }
        sctp: <sctpType> like ISCTP if numSctpApps>0 {
            @display("p=501,141");
        }
        pingApp: PingApp {
            parameters:
                @display("p=501,230");
        }
    connections allowunconnected:
        for i=0..numTcpApps-1 {
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }

        tcp.ipOut --> networkLayer.tcpIn if numTcpApps>0;
        tcp.ipIn <-- networkLayer.tcpOut if numTcpApps>0;

        for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }

        udp.ipOut --> networkLayer.udpIn if numUdpApps>0;
        udp.ipIn <-- networkLayer.udpOut if numUdpApps>0;

        for i=0..numSctpApps-1 {
            sctpApp[i].sctpOut --> sctp.from_appl++;
            sctp.to_appl++ --> sctpApp[i].sctpIn;
        }
        sctp.to_ip --> networkLayer.sctpIn if numSctpApps>0;
        networkLayer.sctpOut --> sctp.from_ip if numSctpApps>0;

        networkLayer.pingOut --> pingApp.pingIn;
        networkLayer.pingIn <-- pingApp.pingOut;
}
