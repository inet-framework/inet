//
// Copyright (C) 2004 Andras Varga
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

moduleinterface IIpv4
{
    gates:
        input transportIn @labels(Ipv4ControlInfo/down,TcpHeader,UdpHeader,SctpHeader);
        output transportOut @labels(Ipv4ControlInfo/up,TcpHeader,UdpHeader,SctpHeader);
        input queueIn @labels(Ipv4Header,ArpPacket,Ieee802Ctrl);
        output queueOut @labels(Ipv4Header,ArpPacket,Ieee802Ctrl);
}

//
// Implements the IPv4 protocol. The protocol header is represented
// by the ~Ipv4Header message class.
//
// <b>Interfacing with higher layer protocols</b>
//
// To send a packet over IPv4 from a higher layer protocol, the module should
// fill in an ~L3AddressReq object, attach it to the packet with the Packets's
// addTag() method, then send the packet to the ~Ipv4 module.
//
// When ~Ipv4 sends up a packet to a higher layer protocol, it will also attach
// an ~L3AddressInd to the packet, with the source and destination IPv4 addresses
// of the IPv4 datagram in which the packet arrived.
//
// ~Ipv4 can serve several higher-layer protocols. The higher layer protocols
// should call registerProtocol with their gate towards the ~Ipv4 module,
// for fill up the protocol-to-gateindex map. When delivering packets to them,
// the output gate is determined from the Protocol in the IPv4 header.
//
// <b>Routing and interfacing with lower layers</b>
//
// The routing table is stored in the module ~Ipv4RoutingTable. When a datagram
// needs to be routed, ~Ipv4 queries ~Ipv4RoutingTable for the output interface
// (or "port") and next hop address of the packet. This is done by directly
// calling C++ methods (such as findBestMatchingRoute(destAddress)) of ~Ipv4RoutingTable.
// No message exchange with ~Ipv4RoutingTable takes place.
//
// A routed datagram will be sent to the queueOut, which is expected to be
// connected to ~INetworkInterface modules.
//
// Routing protocol implementations (e.g. OSPF and ISIS) can also query
// and manipulate the route table by calling ~Ipv4RoutingTable's methods in C++.
//
// <b>Working with Arp</b>
//
// Ipv4 module subscribe to arpResolutionCompleted and arpResolutionFailed signals on Arp module.
// The ~Arp module accessed via arpOut gate, should not insert any module between ~Ipv4 and ~Arp.
// Before Ipv4 module send down a packet to lower layer, ask MacAddress of next hop from Arp via
// method call. If MacAddress unspecified, then start address resolution via Arp method call and
// insert packet to a queue specified by next hop addr.
// When received a arpResolutionCompleted, then send packets from queue of next hop addr.
// When received a arpResolutionFailed, then drop packets from queue of next hop addr.
// When Ipv4 module received an ARP packet from Lower Layer on some queueIn gate,
// then send out this packet on arpOut gate. When received a packet on arpIn gate,
// then send out this packet on the specified queueOut gate.
//
// <b>Performance model, QoS</b>
//
// In the current form, ~Ipv4 contains a FIFO which queues up Ipv4 datagrams;
// datagrams are processed in order. The processing time is determined by the
// procDelay module parameter.
//
// The current performance model comes from the QueueBase C++ base class.
// If you need a more sophisticated performance model, you may change the
// module implementation (the Ipv4 class), and: (1) override the startService()
// method which determines processing time for a packet, or (2) use a
// different base class.
//
// @see ~Ipv4RoutingTable, ~Arp
//
// @author Andras Varga
//
simple Ipv4 like IIpv4
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string arpModule;
        string icmpModule;
        string crcMode @enum("declared", "computed") = default("declared");
        double procDelay @unit(s) = default(0s);
        int timeToLive = default(32);
        int multicastTimeToLive = default(32);
        double fragmentTimeout @unit(s) = default(60s);
        bool limitedBroadcast = default(false); // send out limited broadcast packets comming from higher layer
        string directBroadcastInterfaces = default("");   // list of interfaces that direct broadcast is enabled (by default direct broadcast is disabled on all interfaces)
        @display("i=block/routing");
        @signal[packetSentToUpper](type=cPacket);
        @signal[packetReceivedFromUpper](type=cPacket);
        @signal[packetSentToLower](type=cPacket);
        @signal[packetReceivedFromLower](type=cPacket);
        @signal[packetDropped](type=cPacket);
        @signal[ipv4NewMulticast](type=inet::Ipv4Header);
        @signal[ipv4DataOnNonrpf](type=inet::Ipv4Header);
        @signal[ipv4DataOnRpf](type=inet::Ipv4Header);
        @signal[ipv4MdataRegister](type=inet::Packet);
        @statistic[packetDropAddressResolutionFailed](title="packet drop: address resolution failed"; source=packetDropReasonIsAddressResolutionFailed(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropHopLimitReached](title="packet drop: hop limit reached"; source=packetDropReasonIsHopLimitReached(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropForwardingDisabled](title="packet drop: forwarding disabled"; source=packetDropReasonIsForwardingDisabled(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropNoInterfaceFound](title="packet drop: no interface found"; source=packetDropReasonIsNoInterfaceFound(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropNoRouteFound](title="packet drop: no route found"; source=packetDropReasonIsNoRouteFound(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropUndefined](title="packet drop: undefined"; source=packetDropReasonIsUndefined(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    gates:
        input transportIn @labels(Ipv4ControlInfo/down,TcpHeader,UdpHeader,SctpHeader);
        output transportOut @labels(Ipv4ControlInfo/up,TcpHeader,UdpHeader,SctpHeader);
        input queueIn @labels(Ipv4Header,ArpPacket,Ieee802Ctrl);
        output queueOut @labels(Ipv4Header,ArpPacket,Ieee802Ctrl);
}

