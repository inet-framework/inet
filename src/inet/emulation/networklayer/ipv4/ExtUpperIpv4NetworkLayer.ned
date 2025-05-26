//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.emulation.networklayer.ipv4;

import inet.common.MessageDispatcher;
import inet.networklayer.configurator.ipv4.Ipv4NodeConfigurator;
import inet.networklayer.contract.IArp;
import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.ipv4.IIpv4;
import inet.networklayer.ipv4.Ipv4RoutingTable;


//
// Provides a network layer that connects the simulation to a real network on the
// host computer using a TUN device. Allows simulated applications to send and
// receive packets through the host's networking stack at the IP layer. Contains
// standard components like routing table and ARP, and uses ~ExtUpperIpv4 with a
// TUN device for external connectivity.
//
// Using this network layer in a host or router implies that layers above IPv4 are
// provided by the host operating system, while layers below IPv4 are simulated.
//
module ExtUpperIpv4NetworkLayer like INetworkLayer
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.routingTableModule = default(absPath(".routingTable"));
        @display("i=block/fork");

    gates:
        input ifIn @labels(INetworkHeader);
        output ifOut @labels(INetworkHeader);
        input transportIn @labels(Ipv4ControlInfo/down);
        output transportOut @labels(Ipv4ControlInfo/up);

    submodules:
        configurator: Ipv4NodeConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        routingTable: Ipv4RoutingTable {
            parameters:
                @display("p=100,200;is=s");
        }
        ip: <default("ExtUpperIpv4")> like IIpv4 {
            parameters:
                @display("p=500,100");
        }
        arp: <default("Arp")> like IArp {
            parameters:
                @display("p=200,100;q=pendingQueue");
        }
        lp: MessageDispatcher {
            parameters:
                @display("p=350,200;b=600,5");
        }

    connections allowunconnected:
        arp.ifOut --> lp.in++;
        arp.ifIn <-- lp.out++;

        ip.queueOut --> lp.in++;
        ip.queueIn <-- lp.out++;

        lp.out++ --> { @display("m=s"); } --> ifOut;
        lp.in++ <-- { @display("m=s"); } <-- ifIn;
}

