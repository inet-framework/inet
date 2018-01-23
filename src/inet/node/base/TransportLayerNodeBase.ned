//
// Copyright (C) OpenSim Ltd.
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

package inet.node.base;

import inet.common.MessageDispatcher;
import inet.transportlayer.contract.ISctp;
import inet.transportlayer.contract.ITcp;
import inet.transportlayer.contract.IUdp;

module TransportLayerNodeBase extends NetworkLayerNodeBase
{
    parameters:
        bool hasUdp = default(true);
        bool hasTcp = default(true);
        bool hasSctp = default(false);
        string tcpType = default(firstAvailableOrEmpty("Tcp", "TcpLwip", "TcpNsc"));  // tcp implementation (e.g. ~Tcp, ~TcpLwip, ~TcpNsc) or ~TcpSpoof
        string udpType = default(firstAvailableOrEmpty("Udp"));
        string sctpType = default(firstAvailableOrEmpty("Sctp"));
        @figure[transportLayer](type=rectangle; pos=250,210; size=670,180; fillColor=#ff0000; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[transportLayer.title](type=text; pos=910,220; anchor=ne; text="transport layer");

    submodules:
        udp: <udpType> like IUdp if hasUdp {
            parameters:
                @display("p=400,300");
        }
        tcp: <tcpType> like ITcp if hasTcp {
            parameters:
                @display("p=500,300");
        }
        sctp: <sctpType> like ISctp if hasSctp {
            parameters:
                @display("p=600,300");
        }
        tn: MessageDispatcher {
            parameters:
                @display("p=585,400;b=670,5,,,,1");
        }

    connections allowunconnected:
        udp.ipOut --> tn.in++ if hasUdp;
        udp.ipIn <-- tn.out++ if hasUdp;

        tcp.ipOut --> tn.in++ if hasTcp;
        tcp.ipIn <-- tn.out++ if hasTcp;

        sctp.ipOut --> tn.in++ if hasSctp;
        tn.out++ --> sctp.ipIn if hasSctp;

        tn.out++ --> ipv4.transportIn if hasIpv4;
        tn.in++ <-- ipv4.transportOut if hasIpv4;

        tn.out++ --> ipv6.transportIn if hasIpv6;
        tn.in++ <-- ipv6.transportOut if hasIpv6;

        tn.out++ --> generic.transportIn if hasGn;
        tn.in++ <-- generic.transportOut if hasGn;

        tn.out++ --> nl.in++;
        tn.in++ <-- nl.out++;
}
