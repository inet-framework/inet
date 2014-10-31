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

package inet.examples.bgpv4.BGPUpdate;

import inet.common.misc.MessageChecker;
import inet.linklayer.IWiredNic;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.IPv4NetworkLayer;
import inet.networklayer.ipv4.IPv4RoutingTable;
import inet.routing.bgpv4.BGPRouting;
import inet.routing.ospfv2.OSPFRouting;
import inet.transportlayer.ITCP;


//
// Example IP router with BGPv4 and OSPFv4 support.
//
module BGPRouterEx
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=abstract/router");
        string tcpType = default(firstAvailable("TCP","TCP_lwIP","TCP_NSC","TCP_None"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".routingTable"));
    gates:
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=52,108;i=block/table_s");
        }
        routingTable: IPv4RoutingTable {
            parameters:
                forwarding = true;
                routerId = "auto";
                @display("p=52,172;i=block/table2_s");
        }
        ospf: OSPFRouting {
            parameters:
                @display("p=235,284,row;i=block/network2");
        }
        bgp: BGPRouting {
            parameters:
                ospfRoutingModule = "^.ospf";
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
        tcp: <tcpType> like ITCP {
            parameters:
                @display("p=160,202;i=block/transport");
        }
        networkLayer: IPv4NetworkLayer {
            parameters:
                proxyARP = true;
                @display("p=120,284;i=block/layer;q=queue");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=74,369,row,110;q=l2queue;i=block/ifcard");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                @display("p=196,369,row,110;q=l2queue;i=block/ifcard");
        }
    connections allowunconnected:
        bgp.tcpOut --> { @display("m=s"); } --> snifferOut.in;
        bgp.tcpIn <-- { @display("m=s"); } <-- snifferIn.out;

        snifferOut.out --> { @display("m=s"); } --> tcp.appIn++;
        snifferIn.in <-- { @display("m=s"); } <-- tcp.appOut++;

        tcp.ipOut --> { @display("m=s"); } --> networkLayer.transportIn++;
        tcp.ipIn <-- { @display("m=s"); } <-- networkLayer.transportOut++;

        ospf.ipOut --> { @display("m=s"); } --> networkLayer.transportIn++;
        ospf.ipIn <-- { @display("m=s"); } <-- networkLayer.transportOut++;

        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].upperLayerOut --> networkLayer.ifIn[i];
            ppp[i].upperLayerIn <-- networkLayer.ifOut[i];
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].upperLayerOut --> networkLayer.ifIn[sizeof(pppg)+i];
            eth[i].upperLayerIn <-- networkLayer.ifOut[sizeof(pppg)+i];
        }
}

