//
// Copyright (C) 2012 OpenSim Ltd
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
// @author Zoltan Bojthe
//

package inet.node.internetcloud;


import inet.common.packet.PcapRecorder;
import inet.linklayer.IWiredNic;
import inet.networklayer.arp.ipv4.ARP;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.configurator.ipv4.IPv4NodeConfigurator;
import inet.networklayer.internetcloud.ICloudDelayer;
import inet.networklayer.ipv4.ErrorHandling;
import inet.networklayer.ipv4.ICMP;
import inet.networklayer.ipv4.IPv4;
import inet.networklayer.ipv4.IPv4RoutingTable;


//
// This module is an IPv4 router that can delay or drop packets (while retaining their order)
// based on which interface card the packet arrived on and on which interface
// it is leaving the cloud. The delayer module is replacable.
//
// By default the delayer module is ~MatrixCloudDelayer which lets you configure
// the delay, drop and datarate parameters in an XML file. Packet flows, as defined
// by incoming and outgoing interface pairs, are independent of each other.
//
// The ~InternetCloud module can be used only to model the delay between two hops, but
// it is possible to build more complex networks using several ~InternetCloud modules.
//
// @see MatrixCloudDelayer
//
module InternetCloud
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=misc/cloud");
        int numPcapRecorders = default(0); // no of PcapRecorders.
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".routingTable"));
    gates:
        inout pppg[] @labels(PPPFrame-conn);
    types:
        module InternetCloudNetworkLayer
        {
            parameters:
                @display("i=block/fork");
                bool proxyARP = default(true);
                string interfaceTableModule;
                string routingTableModule;
                *.interfaceTableModule = default(absPath(interfaceTableModule));
                *.routingTableModule = default(absPath(routingTableModule));
                *.arpModule = default(absPath(".arp"));
                *.icmpModule = default(absPath(".icmp"));
            gates:
                input ifIn[] @labels(IPDatagram);
                output ifOut[];
            submodules:
                configurator: IPv4NodeConfigurator;
                ip: IPv4 {
                    parameters:
                        timeToLive = 32;
                        multicastTimeToLive = 32;
                        fragmentTimeout = 60s;
                        useProxyARP = true;
                        routingTableModule = "^.^.routingTable";
                        @display("p=85,95;q=queue");
                    gates:
                        transportIn[1];
                        transportOut[1];
                        queueIn[sizeof(ifIn)];
                        queueOut[sizeof(ifOut)];
                }

                delayer: <default("MatrixCloudDelayer")> like ICloudDelayer {
                    @display("p=160,123");
                }
                arp: ARP {
                    parameters:
                        respondToProxyARP = proxyARP;
                        routingTableModule = "^.^.routingTable";
                        @display("p=163,206;q=pendingQueue");
                }
                icmp: ICMP {
                    parameters:
                        @display("p=160,33");
                }
                errorHandling: ErrorHandling {
                    parameters:
                        @display("p=239,33");
                }
            connections allowunconnected:
                // transport Layer
                ip.transportOut[0] --> icmp.localIn;
                ip.transportIn[0] <-- icmp.sendOut;

                icmp.errorOut --> errorHandling.in;

                ip.arpOut --> arp.netwIn;
                ip.arpIn <-- arp.netwOut;

                for i=0..sizeof(ifIn)-1 {
                    ip.queueIn[i] <-- { @display("m=s"); } <-- ifIn[i];
                }
                for i=0..sizeof(ifOut)-1 {
                    ip.queueOut[i] --> { @display("m=s"); } --> ifOut[i];
                }
        }
    submodules:
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=60,131");
        }
        routingTable: IPv4RoutingTable {
            parameters:
                forwarding = true;
                routerId = "auto";
                @display("p=60,196");
        }
        pcapRecorder[numPcapRecorders]: PcapRecorder {
            @display("p=291,60");
        }
        networkLayer: InternetCloudNetworkLayer {
            parameters:
                @display("p=267,157;q=queue");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=125,257,row,110;q=l2queue");
        }
    connections allowunconnected:
        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].upperLayerOut --> networkLayer.ifIn[i];
            ppp[i].upperLayerIn <-- networkLayer.ifOut[i];
        }
}

