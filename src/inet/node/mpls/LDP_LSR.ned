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
package inet.node.mpls;

import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.IWiredNic;
import inet.linklayer.loopback.LoopbackInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.IPv4NetworkLayer;
import inet.networklayer.ipv4.IPv4RoutingTable;
import inet.networklayer.ldp.LDP;
import inet.networklayer.mpls.LIBTable;
import inet.networklayer.mpls.MPLS;
import inet.networklayer.ted.LinkStateRouting;
import inet.networklayer.ted.TED;
import inet.transportlayer.contract.ITCP;
import inet.transportlayer.contract.IUDP;


//
// An LDP-capable router.
//
// LDP Capable Routers are the main building blocks for an LDP MPLS network.
//
module LDP_LSR
{
    parameters:
        @networkNode();
        @labels(node);
        @display("i=abstract/router");
        bool hasStatus = default(false);
        double holdTime @unit("s") = default(15s);
        double helloInterval @unit("s") = default(5s);
        string peers;
        string routerId = default("auto");
        string tcpType = default(firstAvailable("TCP","TCP_lwIP","TCP_NSC"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".routingTable"));
        *.tedModule = default(absPath(".ted"));
        *.libTableModule = default(absPath(".libTable"));
    gates:
        inout pppg[] @labels(PPPFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=73,45");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=73,176;is=s");
        }
        routingTable: IPv4RoutingTable {
            parameters:
                forwarding = true;
                routerId = routerId;
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
        tcp: <tcpType> like ITCP {
            parameters:
                @display("p=177,145");
        }
        udp: <default(firstAvailable("UDP"))> like IUDP {
            parameters:
                @display("p=267,145");
        }
        networkLayer: IPv4NetworkLayer {
            parameters:
                @display("p=224,223");
            gates:
                ifIn[sizeof(pppg)+1];
                ifOut[sizeof(pppg)+1];
        }
        lo0: LoopbackInterface {
            @display("p=50,388");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=131,388,row,90;q=l2queue");
        }
        mpls: MPLS {
            parameters:
                //peers = peers,
                classifierModule = "^.ldp";
                @display("p=224,302");
            gates:
                netwIn[sizeof(pppg)+1];
                netwOut[sizeof(pppg)+1];
                ifIn[sizeof(pppg)+1];
                ifOut[sizeof(pppg)+1];
        }
        libTable: LIBTable {
            parameters:
                @display("p=340,305");
        }
    connections allowunconnected:
        linkStateRouting.ipOut --> networkLayer.transportIn++;
        linkStateRouting.ipIn <-- networkLayer.transportOut++;

        ldp.tcpIn <-- tcp.appOut++;
        ldp.tcpOut --> tcp.appIn++;

        ldp.udpOut --> udp.appIn++;
        ldp.udpIn <-- udp.appOut++;

        udp.ipOut --> networkLayer.transportIn++;
        udp.ipIn <-- networkLayer.transportOut++;

        tcp.ipOut --> networkLayer.transportIn++;
        tcp.ipIn <-- networkLayer.transportOut++;

        lo0.upperLayerOut --> networkLayer.ifIn[0];
        networkLayer.ifOut[0] --> lo0.upperLayerIn;

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;

            ppp[i].upperLayerOut --> mpls.ifIn[i+1];
            ppp[i].upperLayerIn <-- mpls.ifOut[i+1];

            mpls.netwOut[i+1] --> networkLayer.ifIn[i+1];
            mpls.netwIn[i+1] <-- networkLayer.ifOut[i+1];
        }
}
