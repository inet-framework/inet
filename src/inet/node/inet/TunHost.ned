//
// Copyright (C) 2015 Irene Ruengeler
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

package inet.node.inet;

import inet.transportlayer.contract.ITCP;
import inet.transportlayer.contract.IUDP;
import inet.transportlayer.contract.ISCTP;
import inet.linklayer.tun.TunInterface;
import inet.applications.contract.ITunApp;


//
// IPv4 host with SCTP, TCP, UDP layers and a packetdrill application.
// The application has a direct connection to the TunInterface.
//
module TunHost extends NodeBase
{
    parameters:
        @display("i=device/pc2;bgl=4;bgb=588,447");
        int numTcpTunApps = default(0);    // no of TCP packetdrill apps.
        int numUdpTunApps = default(0);    // no of UDP packetdrill apps.
        int numSctpTunApps = default(0);   // no of SCTP packetdrill apps.
        bool hasTcpTun = default(numTcpTunApps>0);
        bool hasUdpTun = default(numUdpTunApps>0);
        bool hasSctpTun = default(numSctpTunApps>0);
        string tcpType = default(firstAvailableOrEmpty("TCP", "TCP_lwIP", "TCP_NSC"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        string udpType = default(firstAvailableOrEmpty("UDP"));
        string sctpType = default(firstAvailableOrEmpty("SCTP"));
    submodules:
        tunApp: <> like ITunApp {
            parameters:
                @display("p=280,40");
        }
        tcp: <tcpType> like ITCP if hasTcpTun {
            parameters:
                @display("p=230,140");
        }
        udp: <udpType> like IUDP if hasUdpTun {
            parameters:
                @display("p=330,140");
        }
        sctp: <sctpType> like ISCTP if hasSctpTun {
            parameters:
                @display("p=430,140");
        }

    connections allowunconnected:
        tunApp.tcpOut --> tcp.appIn++ if hasTcpTun;
        tunApp.tcpIn <-- tcp.appOut++ if hasTcpTun;

        tcp.ipOut --> networkLayer.transportIn++ if hasTcpTun;
        tcp.ipIn <-- networkLayer.transportOut++ if hasTcpTun;

        tunApp.udpOut --> udp.appIn++ if hasUdpTun;
        tunApp.udpIn <-- udp.appOut++ if hasUdpTun;

        udp.ipOut --> networkLayer.transportIn++ if hasUdpTun;
        udp.ipIn <-- networkLayer.transportOut++ if hasUdpTun;

        tunApp.sctpOut --> sctp.from_appl++ if hasSctpTun;
        tunApp.sctpIn <-- sctp.to_appl++ if hasSctpTun;

        sctp.to_ip --> networkLayer.transportIn++ if hasSctpTun;
        sctp.from_ip <-- networkLayer.transportOut++ if hasSctpTun;

        tunApp.tunOut --> tun.appIn;
        tunApp.tunIn <-- tun.appOut;
}
