//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.bgpv4.BgpUpdate;

import inet.common.MessageDispatcher;
import inet.common.misc.MessageChecker;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.IPppInterface;
import inet.linklayer.ethernet.contract.IEthernetLayer;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.Ipv4NetworkLayer;
import inet.routing.bgpv4.Bgp;
import inet.routing.ospfv2.Ospfv2;
import inet.transportlayer.contract.ITcp;


//
// Example IP router with BGPv4 and OSPFv4 support.
//
module BGPRouterEx
{
    parameters:
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=abstract/router");
        bool forwarding = true;
        bool multicastForwarding = default(false);
        ethernet.registerProtocol = default(true);
        *.forwarding = this.forwarding;
        *.multicastForwarding = this.multicastForwarding;
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".ipv4.routingTable"));
    gates:
        inout pppg[] @labels(PppFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=100,100;i=block/table_s");
        }
        ospf: Ospfv2 {
            parameters:
                @display("p=400,240,row;i=block/network2");
        }
        bgp: Bgp {
            parameters:
                ospfRoutingModule = "^.ospf";
                @display("p=250,80;i=block/network");
        }
        snifferIn: MessageChecker {
            parameters:
                @display("i=block/uparrow_s;p=175,160");
        }
        snifferOut: MessageChecker {
            parameters:
                @display("i=block/downarrow_s;p=325,160");
        }
        tcp: <default(firstAvailable("Tcp","TcpLwip","TcpNsc"))> like ITcp {
            parameters:
                @display("p=250,240;i=block/transport");
        }
        tn: MessageDispatcher {
            parameters:
                @display("b=400,5,,,,1;p=325,320");
        }
        ipv4: Ipv4NetworkLayer {
            parameters:
                routingTable.routerId = "auto";
                @display("p=250,400;i=block/layer;q=queue");
        }
        nl: MessageDispatcher {
            parameters:
                @display("b=400,5,,,,1;p=325,480");
        }
        ethernet: <default("EthernetEncapsulation")> like IEthernetLayer if typename != "" {
            @display("p=250,560");
        }
        li: MessageDispatcher {
            parameters:
                @display("b=400,5,,,,1;p=325,640");
        }
        ppp[sizeof(pppg)]: <default("PppInterface")> like IPppInterface {
            parameters:
                @display("p=250,720,row,110;q=l2queue;i=block/ifcard");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IEthernetInterface {
            parameters:
                @display("p=400,720,row,110;q=l2queue;i=block/ifcard");
        }
    connections allowunconnected:
        bgp.socketOut --> { @display("m=s"); } --> snifferOut.in;
        bgp.socketIn <-- { @display("m=s"); } <-- snifferIn.out;

        snifferOut.out --> { @display("m=s"); } --> tcp.appIn;
        snifferIn.in <-- { @display("m=s"); } <-- tcp.appOut;

        tcp.ipOut --> { @display("m=s"); } --> tn.in++;
        tcp.ipIn <-- { @display("m=s"); } <-- tn.out++;
        ospf.ipOut --> { @display("m=s"); } --> tn.in++;
        ospf.ipIn <-- { @display("m=s"); } <-- tn.out++;

        tn.out++ --> { @display("m=s"); } --> ipv4.transportIn;
        tn.in++ <-- { @display("m=s"); } <-- ipv4.transportOut;

        ipv4.ifOut --> { @display("m=s"); } --> nl.in++;
        ipv4.ifIn <-- { @display("m=s"); } <-- nl.out++;

        nl.out++ --> li.in++;
        li.out++ --> nl.in++;

        nl.out++ --> ethernet.upperLayerIn if exists(ethernet);
        ethernet.upperLayerOut --> nl.in++ if exists(ethernet);
        ethernet.lowerLayerOut --> li.in++ if exists(ethernet);
        li.out++ --> ethernet.lowerLayerIn if exists(ethernet);

        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].upperLayerOut --> li.in++;
            ppp[i].upperLayerIn <-- li.out++;
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].upperLayerOut --> li.in++;
            eth[i].upperLayerIn <-- li.out++;
        }
}

