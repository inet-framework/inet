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

package inet.nodes.inet;

import inet.base.NotificationBoard;
import inet.linklayer.ethernet.EthernetInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.nodes.inet.NetworkLayer;
import inet.transport.tcp.TCP;
import inet.networklayer.bgpv4.BGPRouting;
import inet.linklayer.ppp.PPPInterfaceNoQueue;
import inet.networklayer.ospfv2.OSPFRouting;

//
// \IP router with BGPv4 and OSPFv4 support.
//
module BGPRouter
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=abstract/router");
        string routingFile = default("");
    gates:
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
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
        ospf: OSPFRouting {
            parameters:
                @display("p=252,188,row;i=block/network2");
        }
        bgp: BGPRouting {
            parameters:
                @display("p=168,42;i=block/network");
        }
        tcp: TCP {
            parameters:
                @display("p=160,194;i=block/transport");
        }
        networkLayer: NetworkLayer {
            parameters:
                proxyARP = true;
                @display("p=216,300;i=block/layer;q=queue");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
        ppp[sizeof(pppg)]: PPPInterfaceNoQueue {
            parameters:
                @display("p=74,369,row,110;q=l2queue;i=block/ifcard");
        }
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=145,257,row,110;q=l2queue");
        }
    connections allowunconnected:
        bgp.toTCP --> tcp.appIn++;
        bgp.fromTCP <-- tcp.appOut++;

        tcp.ipOut --> networkLayer.tcpIn;
        tcp.ipIn <-- networkLayer.tcpOut;

        ospf.ipOut --> networkLayer.ospfIn;
        ospf.ipIn <-- networkLayer.ospfOut;

        // connections to network outside		
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        }
        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].netwOut --> networkLayer.ifIn[sizeof(pppg)+i];
            eth[i].netwIn <-- networkLayer.ifOut[sizeof(pppg)+i];
        }
}

