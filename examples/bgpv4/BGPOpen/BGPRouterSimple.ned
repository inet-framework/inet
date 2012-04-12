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

import inet.util.MessageChecker;
import inet.transport.ITCP;
import inet.nodes.inet.NetworkLayer;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.bgpv4.BGPRouting;
import inet.linklayer.IWiredNic;
import inet.base.NotificationBoard;


//
// Example IPv4 router with BGPv4 support.
//
module BGPRouterSimple
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=abstract/router");
        string routingFile;
        string tcpType = default(firstAvailable("TCP","TCP_lwIP","TCP_NSC","TCP_None"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
    gates:
        inout pppg[] @labels(PPPFrame-conn);
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=52,44;i=block/control_s");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=52,108;i=block/table_s");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = true;
                routerId = "auto";
                routingFile = routingFile;
                @display("p=52,172;i=block/table2_s");
        }
        bgp: BGPRouting {
            parameters:
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
        networkLayer: NetworkLayer {
            parameters:
                proxyARP = true;
                @display("p=120,284;i=block/layer;q=queue");
            gates:
                ifIn[sizeof(pppg)];
                ifOut[sizeof(pppg)];
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=74,369,row,110;q=l2queue;i=block/ifcard");
        }
    connections allowunconnected:
        bgp.tcpOut --> { @display("m=s"); } --> snifferOut.in;
        bgp.tcpIn <-- { @display("m=s"); } <-- snifferIn.out;

        snifferOut.out --> { @display("m=s"); } --> tcp.appIn++;
        snifferIn.in <-- { @display("m=s"); } <-- tcp.appOut++;

        tcp.ipOut --> { @display("m=s"); } --> networkLayer.tcpIn;
        tcp.ipIn <-- { @display("m=s"); } <-- networkLayer.tcpOut;

        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> { @display("m=s"); } <--> ppp[i].phys;
            ppp[i].upperLayerOut --> networkLayer.ifIn[i];
            ppp[i].upperLayerIn <-- networkLayer.ifOut[i];
        }
}

