//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

package inet.networklayer.ipv4;

import inet.common.MessageDispatcher;
import inet.networklayer.contract.IARP;
import inet.networklayer.configurator.ipv4.IPv4NodeConfigurator;
import inet.networklayer.contract.INetworkLayer;


//
// Network layer of an IPv4 node.
//
// Interfaces to transport layer: TCP, UDP, echo/ping, RSVP
//
module IPv4NetworkLayer like INetworkLayer
{
    parameters:
        bool proxyARP = default(true);
        bool forwarding = default(false);
        bool multicastForwarding = default(false);
        string igmpType = default("IGMPv2");
        string arpType = default("ARP");
        string interfaceTableModule;
        *.forwarding = forwarding;
        *.multicastForwarding = multicastForwarding;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.routingTableModule = default(absPath(".routingTable"));
        *.arpModule = default(absPath(".arp"));
        *.icmpModule = default(absPath(".icmp"));
        arp.respondToProxyARP = proxyARP;
        @display("i=block/fork");
    gates:
        input ifIn @labels(INetworkDatagram);
        output ifOut @labels(INetworkDatagram);
        input transportIn @labels(IPv4ControlInfo/down);
        output transportOut @labels(IPv4ControlInfo/up);
        input igmpIn;
        output igmpOut;

    submodules:
        configurator: IPv4NodeConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        routingTable: IPv4RoutingTable {
            parameters:
                @display("p=100,200;is=s");
        }
        ip: IPv4 {
            parameters:
                useProxyARP = true; // as routes with unspecified next-hop addr are quite common
                @display("p=450,400;q=queue");
        }
        arp: <arpType> like IARP {
            parameters:
                @display("p=300,400;q=pendingQueue");
        }
        icmp: ICMP {
            parameters:
                @display("p=600,200");
        }
        dp: MessageDispatcher {
            parameters:
                @display("p=450,300;b=480,5");
        }
        igmp: <igmpType> like IIGMP {
            parameters:
                @display("p=300,200");
        }
        ifdp: MessageDispatcher {
            parameters:
                @display("p=450,500;b=480,5");
        }
        tn: MessageDispatcher {
            parameters:
                @display("p=450,100;b=480,5");
        }

    connections allowunconnected:
        dp.upperLayerOut++ --> { @display("m=n"); } --> tn.lowerLayerIn++;
        tn.lowerLayerOut++ --> { @display("m=n"); } --> dp.upperLayerIn++;


        dp.upperLayerOut++ --> icmp.ipIn;
        dp.upperLayerIn++ <-- icmp.ipOut;

        dp.upperLayerOut++ --> igmp.ipIn;
        dp.upperLayerIn++ <-- igmp.ipOut;

        ip.transportOut --> dp.lowerLayerIn++;
        ip.transportIn <-- dp.lowerLayerOut++;

        arp.ifOut --> ifdp.upperLayerIn++;
        arp.ifIn <-- ifdp.upperLayerOut++;

        igmp.routerOut --> tn.lowerLayerIn++;
        igmp.routerIn <-- tn.lowerLayerOut++;

        ip.queueIn <-- ifdp.upperLayerOut++;
        ip.queueOut --> ifdp.upperLayerIn++;

        ifdp.lowerLayerIn++ <-- { @display("m=s"); } <-- ifIn;
        ifdp.lowerLayerOut++ --> { @display("m=s"); } --> ifOut;

        tn.lowerLayerOut++ --> { @display("m=n"); } --> icmp.transportIn;
        icmp.transportOut --> { @display("m=n"); } --> tn.lowerLayerIn++;

        tn.upperLayerOut++ --> transportOut;
        transportIn --> tn.upperLayerIn++;
}

