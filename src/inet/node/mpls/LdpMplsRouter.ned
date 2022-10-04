//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
        *.forwarding = true;
        *.routingTable.routerId = this.routerId;
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".ipv4.routingTable"));
        *.tedModule = default(absPath(".ted"));
        *.libTableModule = default(absPath(".libTable"));
    gates:
        inout pppg[] @labels(PppFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=100,300;is=s");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=100,100;is=s");
        }
        ldp: Ldp {
            parameters:
                holdTime = parent.holdTime;
                helloInterval = parent.helloInterval;
                @display("p=450,80");
        }
        at: MessageDispatcher {
            parameters:
                @display("p=450,160;b=153,5,,,,1");
        }
        ted: Ted {
            parameters:
                @display("p=100,400;is=s");
        }
        linkStateRouting: LinkStateRouting {
            parameters:
                peers = parent.peers;
                @display("p=900,80");
        }
        tcp: <default(firstAvailable("Tcp","TcpLwip","TcpNsc"))> like ITcp {
            parameters:
                @display("p=300,200");
        }
        udp: <default(firstAvailable("Udp"))> like IUdp {
            parameters:
                @display("p=600,200");
        }
        tn: MessageDispatcher {
            parameters:
                @display("p=700,240;b=1000,5,,,,1");
        }
        ipv4: Ipv4NetworkLayer {
            parameters:
                @display("p=500,320");
        }
        nm: MessageDispatcher {
            parameters:
                @display("p=700,400;b=1000,5,,,,1");
        }
        lo[numLoInterfaces]: <default("LoopbackInterface")> like ILoopbackInterface {
            @display("p=250,640");
        }
        ppp[sizeof(pppg)]: <default("PppInterface")> like IPppInterface {
            parameters:
                @display("p=400,640,row,150;q=l2queue");
        }
        mpls: Mpls {
            parameters:
                //peers = parent.peers;
                classifierModule = "^.ldp";
                @display("p=500,480");
        }
        libTable: LibTable {
            parameters:
                @display("p=100,200;is=s");
        }

        ml: MessageDispatcher {
            parameters:
                @display("p=700,560;b=1000,5,,,,1");
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
        nm.out++ --> mpls.upperLayerIn;
        mpls.upperLayerOut --> nm.in++;
        mpls.lowerLayerOut --> ml.in++;
        ml.out++ --> mpls.lowerLayerIn;
}

