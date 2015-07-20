//
// Copyright (C) 2015 OpenSim Ltd.
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

package inet.node.mpls;

import inet.common.MessageDispatcher;
import inet.transportlayer.contract.ISCTP;
import inet.transportlayer.contract.ITCP;
import inet.transportlayer.contract.IUDP;

//
// This module contains the transport layer infrastructure for network nodes.
//
// See derived modules: ~StandardHost, ~WirelessHost, ~AdhocHost, ~Router, etc.
//
module MplsTransportLayerNodeBase extends MplsNetworkLayerNodeBase
{
    parameters:
        bool hasUdp = default(true);
        bool hasTcp = default(true);
        bool hasSctp = default(false);
        string tcpType = default(firstAvailableOrEmpty("TCP", "TCP_lwIP", "TCP_NSC"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        string udpType = default(firstAvailableOrEmpty("UDP"));
        string sctpType = default(firstAvailableOrEmpty("SCTP"));
        *.routingTableModule = default(hasIpv4 ? "^.ipv4.routingTable" : hasIpv6 ? "^.ipv6.routingTable" : hasGn ? "^.gn.routingTable" : "");
        @figure[transportLayer](type=rectangle; pos=250,200; size=600,200; fillColor=#ff0000; cornerRadius=12; fillOpacity=0.1);
        @figure[transportLayer.title](type=text; pos=740,210; text="transport layer");
        @figure[submodules];

    submodules:
        udp: <udpType> like IUDP if hasUdp {
            parameters:
                @display("p=400,300");
        }
        tcp: <tcpType> like ITCP if hasTcp {
            parameters:
                @display("p=500,300");
        }
        sctp: <sctpType> like ISCTP if hasSctp {
            parameters:
                @display("p=600,300");
        }
        tn: MessageDispatcher {
            parameters:
                @display("b=580,5,,,,1;p=550,400");
        }

    connections allowunconnected:
        udp.ipOut --> tn.upperLayerIn++ if hasUdp;
        udp.ipIn <-- tn.upperLayerOut++ if hasUdp;

        tcp.ipOut --> tn.upperLayerIn++ if hasTcp;
        tcp.ipIn <-- tn.upperLayerOut++ if hasTcp;

        sctp.ipOut --> tn.upperLayerIn++ if hasSctp;
        tn.upperLayerOut++ --> sctp.ipIn if hasSctp;

        tn.lowerLayerOut++ --> ipv4.transportIn if hasIpv4;
        tn.lowerLayerIn++ <-- ipv4.transportOut if hasIpv4;

        tn.lowerLayerOut++ --> ipv4.icmpIn if hasIpv4;
        tn.lowerLayerIn++ <-- ipv4.icmpOut if hasIpv4;

        tn.lowerLayerOut++ --> ipv4.igmpIn if hasIpv4;
        tn.lowerLayerIn++ <-- ipv4.igmpOut if hasIpv4;

        tn.lowerLayerOut++ --> ipv6.transportIn if hasIpv6;
        tn.lowerLayerIn++ <-- ipv6.transportOut if hasIpv6;

        tn.lowerLayerOut++ --> ipv6.icmpIn if hasIpv6;
        tn.lowerLayerIn++ <-- ipv6.icmpOut if hasIpv6;

        tn.lowerLayerOut++ --> gn.transportIn if hasGn;
        tn.lowerLayerIn++ <-- gn.transportOut if hasGn;

        tn.lowerLayerOut++ --> nl.upperLayerIn++;
        tn.lowerLayerIn++ <-- nl.upperLayerOut++;
}
