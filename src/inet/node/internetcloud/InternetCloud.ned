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


import inet.common.MessageDispatcher;
import inet.common.packet.PcapRecorder;
import inet.linklayer.contract.IWiredNic;
import inet.networklayer.arp.ipv4.ARP;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.configurator.ipv4.IPv4NodeConfigurator;
import inet.networklayer.internetcloud.ICloudDelayer;
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
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=misc/cloud");
        int numPcapRecorders = default(0); // no of PcapRecorders.
        *.interfaceTableModule = default(absPath(".interfaceTable"));
    gates:
        inout pppg[] @labels(PPPFrame-conn);
    types:
        module InternetCloudNetworkLayer
        {
            parameters:
                @display("i=block/fork");
                bool proxyARP = default(true);
                string interfaceTableModule;
                *.interfaceTableModule = default(absPath(interfaceTableModule));
                *.routingTableModule = default(absPath(".routingTable"));
                *.arpModule = default(absPath(".arp"));
                *.icmpModule = default(absPath(".icmp"));
            gates:
                input ifIn @labels(IPDatagram);
                output ifOut;
            submodules:
                configurator: IPv4NodeConfigurator {
                    parameters:
                        @display("p=100,100;is=s");
                }
                ip: IPv4 {
                    parameters:
                        timeToLive = 32;
                        multicastTimeToLive = 32;
                        fragmentTimeout = 60s;
                        useProxyARP = true;
                        @display("p=350,200;q=queue");
                }

                routingTable: IPv4RoutingTable {
                    parameters:
                        forwarding = true;
                        routerId = "auto";
                        @display("p=100,200;is=s");
                }
                delayer: <default("MatrixCloudDelayer")> like ICloudDelayer {
                    parameters:
                        @display("p=450,200");
                }
                arp: ARP {
                    parameters:
                        respondToProxyARP = proxyARP;
                        @display("p=250,200;q=pendingQueue");
                }
                icmp: ICMP {
                    parameters:
                        @display("p=350,108");
                }
                dp: MessageDispatcher {
                    parameters:
                        @display("p=350,286;b=380,5,,,,1");
                }

            connections allowunconnected:
                // transport Layer
                ip.transportOut --> icmp.ipIn;
                ip.transportIn <-- icmp.ipOut;

                arp.netwIn <-- dp.upperLayerOut++;
                arp.netwOut --> dp.upperLayerIn++;

                ip.queueIn <-- { @display("m=s"); } <-- dp.upperLayerOut++;
                ip.queueOut --> { @display("m=s"); } --> dp.upperLayerIn++;

                dp.lowerLayerIn++ <-- { @display("m=s"); } <-- ifIn;
                dp.lowerLayerOut++ --> { @display("m=s"); } --> ifOut;
        }
    submodules:
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=100,100;is=s");
        }
        pcapRecorder[numPcapRecorders]: PcapRecorder {
            parameters:
                @display("p=100,200");
        }
        ipv4: InternetCloudNetworkLayer {
            parameters:
                @display("p=400,100;q=queue");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=400,300,row,110;q=l2queue");
        }
        nl: MessageDispatcher {
            parameters:
                @display("p=400,200;b=380,5,,,,1");
        }

    connections allowunconnected:
        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].upperLayerOut --> nl.lowerLayerIn++;
            nl.lowerLayerOut++ --> ppp[i].upperLayerIn;
        }
        ipv4.ifOut --> nl.upperLayerIn++;
        nl.upperLayerOut++ --> ipv4.ifIn;
}

