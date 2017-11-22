//
// Copyright (C) 2010 Helene Lageber
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.examples.bgpv4.BGPOpen;

import inet.common.MessageDispatcher;
import inet.common.misc.MessageChecker;
import inet.linklayer.contract.IWiredNic;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.IPv4NetworkLayer;
import inet.networklayer.ipv4.Ipv4RoutingTable;
import inet.routing.bgpv4.BgpRouting;
import inet.transportlayer.contract.ITCP;


//
// Example IPv4 router with BGPv4 support.
//
module BGPRouterSimple
{
    parameters:
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=abstract/router");
        string tcpType = default(firstAvailable("TCP","TcpLwip","TcpNsc"));  // tcp implementation (e.g. ~TCP, ~TcpLwip, ~TcpNsc) or ~TcpSpoof
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".routingTable"));
    gates:
        inout pppg[] @labels(PPPFrame-conn);
    submodules:
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=52,108;i=block/table_s");
        }
        routingTable: Ipv4RoutingTable {
            parameters:
                forwarding = true;
                routerId = "auto";
                @display("p=52,172;i=block/table2_s");
        }
        bgp: BgpRouting {
            parameters:
                ospfRoutingModule = "";
                @display("p=160,50;i=block/network");
        }
        snifferIn: MessageChecker {
            parameters:
                @display("i=block/uparrow_s;p=132,124");
        }
        snifferOut: MessageChecker {
            parameters:
                @display("i=block/downarrow_s;p=196,124");
        }
        tcp: <tcpType> like ITCP if tcpType != "" {
            parameters:
                @display("p=160,202;i=block/transport");
        }
        tn: MessageDispatcher {
            parameters:
                @display("b=580,5,,,,1;p=461,274");
        }
        ipv4: IPv4NetworkLayer {
            parameters:
                proxyARP = true;
                @display("p=401,320;i=block/layer;q=queue");
        }
        nl: MessageDispatcher {
            parameters:
                @display("b=580,5,,,,1;p=461,368");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=401,429,row,110;q=l2queue;i=block/ifcard");
        }

    connections allowunconnected:
        bgp.socketOut --> { @display("m=s"); } --> snifferOut.in;
        bgp.socketIn <-- { @display("m=s"); } <-- snifferIn.out;

        snifferOut.out --> { @display("m=s"); } --> tcp.appIn;
        snifferIn.in <-- { @display("m=s"); } <-- tcp.appOut;

        tcp.ipOut --> { @display("m=s"); } --> tn.upperLayerIn++;
        tcp.ipIn <-- { @display("m=s"); } <-- tn.upperLayerOut++;

        tn.lowerLayerOut++ --> { @display("m=s"); } --> ipv4.transportIn;
        tn.lowerLayerIn++ <-- { @display("m=s"); } <-- ipv4.transportOut;

        ipv4.ifOut --> { @display("m=s"); } --> nl.upperLayerIn++;
        ipv4.ifIn <-- { @display("m=s"); } <-- nl.upperLayerOut++;

        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> { @display("m=s"); } <--> ppp[i].phys;
            ppp[i].upperLayerOut --> nl.lowerLayerIn++;
            ppp[i].upperLayerIn <-- nl.lowerLayerOut++;
        }
}

