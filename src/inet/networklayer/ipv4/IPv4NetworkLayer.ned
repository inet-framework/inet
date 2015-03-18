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
        string igmpType = default("IGMPv2");
        string arpType = default("ARP");
        string interfaceTableModule;
        string routingTableModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.routingTableModule = default(absPath(routingTableModule));
        *.arpModule = default(absPath(".arp"));
        *.icmpModule = default(absPath(".icmp"));
        arp.respondToProxyARP = proxyARP;
        @display("i=block/fork");
    gates:
        input ifIn[] @labels(INetworkDatagram);
        output ifOut[] @labels(INetworkDatagram);
        input transportIn[] @labels(IPv4ControlInfo/down);
        output transportOut[] @labels(IPv4ControlInfo/up);
        input pingIn[] @labels(PingPayload/down);
        output pingOut[] @labels(PingPayload/up);
        input igmpIn;
        output igmpOut;

    submodules:
        configurator: IPv4NodeConfigurator {
            @display("p=39,158");
        }
        ip: IPv4 {
            parameters:
                useProxyARP = true; // as routes with unspecified next-hop addr are quite common
                @display("p=85,95;q=queue");
            gates:
                queueIn[sizeof(ifIn)];
                queueOut[sizeof(ifOut)];
        }
        arp: <arpType> like IARP {
            parameters:
                @display("p=165,120;q=pendingQueue;is=s");
        }
        icmp: ICMP {
            parameters:
                @display("p=165,70");
        }
        igmp: <igmpType> like IIGMP {
            parameters:
                @display("p=39,33");
        }
        errorHandling: ErrorHandling {
            parameters:
                @display("p=239,70");
        }

    connections allowunconnected:
        for i=0..sizeof(transportIn)-1 {
            transportIn[i] --> { @display("m=n"); } --> ip.transportIn++;
        }
        for i=0..sizeof(transportOut)-1 {
            ip.transportOut++ --> { @display("m=n"); } --> transportOut[i];
        }

        ip.transportOut++ --> icmp.localIn;
        ip.transportIn++ <-- icmp.sendOut;

        ip.transportOut++ --> igmp.ipIn;
        ip.transportIn++ <-- igmp.ipOut;

        for i=0..sizeof(pingOut)-1 {
            icmp.pingOut++ --> { @display("m=n"); } --> pingOut[i];
        }
        for i=0..sizeof(pingIn)-1 {
            icmp.pingIn++ <-- { @display("m=n"); } <-- pingIn[i];
        }

        icmp.errorOut --> errorHandling.in;

        ip.arpOut --> arp.netwIn;
        ip.arpIn <-- arp.netwOut;

        igmp.routerOut --> igmpOut;
        igmp.routerIn <-- igmpIn;

        for i=0..sizeof(ifIn)-1 {
            ip.queueIn[i] <-- { @display("m=s"); } <-- ifIn[i];
        }
        for i=0..sizeof(ifOut)-1 {
            ip.queueOut[i] --> { @display("m=s"); } --> ifOut[i];
        }
}

