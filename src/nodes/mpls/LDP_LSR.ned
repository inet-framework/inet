//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//




package inet.nodes.mpls;

import inet.base.NotificationBoard;
import inet.linklayer.ppp.PPPInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.ldp.LDP;
import inet.networklayer.mpls.LIBTable;
import inet.networklayer.mpls.MPLS;
import inet.networklayer.ted.LinkStateRouting;
import inet.networklayer.ted.TED;
import inet.nodes.inet.NetworkLayer;
import inet.transport.tcp.TCP;
import inet.transport.udp.UDP;
import inet.util.NAMTraceWriter;


//
// An LDP-capable router.
//
// LDP Capable Routers are main building blocks for an LDP MPLS network.
//
module LDP_LSR
{
    parameters:
        @node();
        @labels(node,mpls-node);
        @display("i=abstract/router");
        double holdTime @unit("s") = default(15s);
        double helloInterval @unit("s") = default(5s);
        int namid = default(-1);
        string peers;
        string routerId = default("auto");
        string routingFile = default("");
    gates:
        inout pppg[] @labels(PPPFrame-conn);
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=70,60");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=70,150");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = true;
                routerId = routerId;
                routingFile = routingFile;
                @display("p=70,240");
        }
        ldp: LDP {
            parameters:
                holdTime = holdTime;
                helloInterval = helloInterval;
                @display("p=222,67");
        }
        ted: TED {
            parameters:
                @display("p=340,244");
        }
        linkStateRouting: LinkStateRouting {
            parameters:
                peers = peers;
                @display("p=360,150");
        }
        tcp: TCP {
            parameters:
                @display("p=177,145");
        }
        udp: UDP {
            parameters:
                @display("p=267,145");
        }
        networkLayer: NetworkLayer {
            parameters:
                @display("p=224,223");
            gates:
                ifIn[sizeof(pppg)];
                ifOut[sizeof(pppg)];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=105,388,row,90;q=l2queue");
        }
        mpls: MPLS {
            parameters:
                //peers = peers,
                classifier = "ldp";
                @display("p=224,302");
            gates:
                netwIn[sizeof(pppg)];
                netwOut[sizeof(pppg)];
                ifIn[sizeof(pppg)];
                ifOut[sizeof(pppg)];
        }
        libTable: LIBTable {
            parameters:
                @display("p=340,305");
        }
        namTrace: NAMTraceWriter {
            parameters:
                namid = namid;
                @display("p=70,310");

        }
    connections allowunconnected:
        linkStateRouting.ipOut --> networkLayer.ospfIn;
        linkStateRouting.ipIn <-- networkLayer.ospfOut;

        ldp.tcpIn <-- tcp.appOut++;
        ldp.tcpOut --> tcp.appIn++;

        ldp.udpOut --> udp.appIn++;
        ldp.udpIn <-- udp.appOut++;

        udp.ipOut --> networkLayer.udpIn;
        udp.ipIn <-- networkLayer.udpOut;

        tcp.ipOut --> networkLayer.tcpIn;
        tcp.ipIn <-- networkLayer.tcpOut;

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;

            ppp[i].netwOut --> mpls.ifIn[i];
            ppp[i].netwIn <-- mpls.ifOut[i];

            mpls.netwOut[i] --> networkLayer.ifIn[i];
            mpls.netwIn[i] <-- networkLayer.ifOut[i];
        }
}



