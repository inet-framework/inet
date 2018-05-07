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

import inet.common.MessageDispatcher;
import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.ILoopbackInterface;
import inet.linklayer.contract.IPppInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.Ipv4NetworkLayer;
import inet.networklayer.ldp.Ldp;
import inet.networklayer.mpls.LibTable;
import inet.networklayer.mpls.Mpls;
import inet.networklayer.ted.LinkStateRouting;
import inet.networklayer.ted.Ted;
import inet.transportlayer.contract.ITcp;
import inet.transportlayer.contract.IUdp;


//
// An LDP-capable router.
//
// LDP Capable Routers are the main building blocks for an LDP MPLS network.
//
module LdpMplsRouter
{
    parameters:
        @networkNode();
        @labels(node);
        @display("i=abstract/router");
        bool hasStatus = default(false);
        double holdTime @unit(s) = default(15s);
        double helloInterval @unit(s) = default(5s);
        int numLoInterfaces = default(1);
        string peers;
        string routerId = default("auto");
        string tcpType = default(firstAvailable("Tcp","TcpLwip","TcpNsc"));  // tcp implementation (e.g. ~Tcp, ~TcpLwip, ~TcpNsc) or ~TcpSpoof
        *.forwarding = true;
        *.routingTable.routerId = routerId;
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".ipv4.routingTable"));
        *.tedModule = default(absPath(".ted"));
        *.libTableModule = default(absPath(".libTable"));
    gates:
        inout pppg[] @labels(PppFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=73,45;is=s");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=73,301;is=s");
        }
        ldp: Ldp {
            parameters:
                holdTime = holdTime;
                helloInterval = helloInterval;
                @display("p=222,67");
        }
        at: MessageDispatcher {
            parameters:
                @display("p=219,107;b=153,5,,,,1");
        }
        ted: Ted {
            parameters:
                @display("p=73,114;is=s");
        }
        linkStateRouting: LinkStateRouting {
            parameters:
                peers = peers;
                @display("p=544,106");
        }
        tcp: <tcpType> like ITcp {
            parameters:
                @display("p=177,145");
        }
        udp: <default(firstAvailable("Udp"))> like IUdp {
            parameters:
                @display("p=267,145");
        }
        tn: MessageDispatcher {
            parameters:
                @display("p=409,185;b=580,5,,,,1");
        }
        ipv4: Ipv4NetworkLayer {
            parameters:
                @display("p=409,223");
        }
        nm: MessageDispatcher {
            parameters:
                @display("p=409,271;b=580,5,,,,1");
        }
        lo[numLoInterfaces]: <default("LoopbackInterface")> like ILoopbackInterface {
            @display("p=147,396");
        }
        ppp[sizeof(pppg)]: <default("PppInterface")> like IPppInterface {
            parameters:
                @display("p=267,396,row,90;q=l2queue");
        }
        mpls: Mpls {
            parameters:
                //peers = peers,
                classifierModule = "^.ldp";
                @display("p=401,309");
        }
        libTable: LibTable {
            parameters:
                @display("p=73,186;is=s");
        }

        ml: MessageDispatcher {
            parameters:
                @display("p=409,350;b=580,5,,,,1");
        }
    connections allowunconnected:
        linkStateRouting.ipOut --> tn.in++;
        tn.out++ --> linkStateRouting.ipIn;

        ldp.socketIn <-- at.out++;
        ldp.socketOut --> at.in++;

        at.out++ --> udp.appIn;
        at.in++ <-- udp.appOut;

        at.out++ --> tcp.appIn;
        at.in++ <-- tcp.appOut;

        udp.ipOut --> tn.in++;
        tn.out++ --> udp.ipIn;

        tcp.ipOut --> tn.in++;
        tn.out++ --> tcp.ipIn;

        for i=0..numLoInterfaces-1 {
            lo[i].upperLayerOut --> nm.in++;
            nm.out++ --> lo[i].upperLayerIn;
        }

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;

            ppp[i].upperLayerOut --> ml.in++;
            ml.out++ --> ppp[i].upperLayerIn;

        }
        ipv4.transportOut --> tn.in++;
        tn.out++ --> ipv4.transportIn;
        ipv4.ifOut --> nm.in++;
        nm.out++ --> ipv4.ifIn;
        nm.out++ --> mpls.netwIn;
        mpls.netwOut --> nm.in++;
        mpls.ifOut --> ml.in++;
        ml.out++ --> mpls.ifIn;
}
