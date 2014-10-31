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

//
// Implements the IPv4 protocol. The protocol header is represented
// by the ~IPv4Datagram message class.
//
// <b>Interfacing with higher layer protocols</b>
//
// To send a packet over IPv4 from a higher layer protocol, the module should
// fill in an ~IPv4ControlInfo object, attach it to the packet with cMessage's
// setControlInfo() method, the send it to the ~IPv4 module.
//
// When ~IPv4 sends up a packet to a higher layer protocol, it will also attach
// an ~IPv4ControlInfo to the packet, with the source and destination IPv4 address,
// etc. of the IPv4 datagram in which the packet arrived.
//
// ~IPv4 can serve several higher-layer protocols. The higher layer protocols should
// send one or more RegisterProtocol message (a simple cMessage with
// ~IPRegisterProtocolCommand controlinfo and kind=IP_C_REGISTER_PROTOCOL)
// to IPv4 module, for fill up the protocol-to-gateindex map.
// When delivering packets to them, the output gate is determined from the Protocol
// field in the IPv4 header.
//
// <b>Routing and interfacing with lower layers</b>
//
// The routing table is stored in the module ~IPv4RoutingTable. When a datagram
// needs to be routed, ~IPv4 queries ~IPv4RoutingTable for the output interface
// (or "port") and next hop address of the packet. This is done by directly
// calling C++ methods (such as findBestMatchingRoute(destAddress)) of ~IPv4RoutingTable.
// No message exchange with ~IPv4RoutingTable takes place.
//
// A routed datagram will be sent to the queueOut gate, with an
// ~IPv4ControlInfo  object attached. queueOut is expected to be
// connected to ~INic modules.
//
// Routing protocol implementations (e.g. OSPF and ISIS) can also query
// and manipulate the route table by calling ~IPv4RoutingTable's methods in C++.
//
// <b>Working with ARP</b>
//
// IPv4 module subscribe to completedARPResolution and failedARPResolution signals on ARP module.
// The ARP module accessed via arpOut gate, should not insert any module between IPv4 and ARP.
// Before IPv4 module send down a packet to lower layer, ask MACAddress of next hop from ARP via
// method call. If MACAddress unspecified, then start address resolution via ARP method call and
// insert packet to a queue specified by next hop addr.
// When received a completedARPResolution, then send packets from queue of next hop addr.
// When received a failedARPResolution, then drop packets from queue of next hop addr.
// When IPv4 module received an ARP packet from Lower Layer on some queueIn gate,
// then send out this packet on arpOut gate. When received a packet on arpIn gate,
// then send out this packet on the specified queueOut gate.
//
// <b>Performance model, QoS</b>
//
// In the current form, ~IPv4 contains a FIFO which queues up IPv4 datagrams;
// datagrams are processed in order. The processing time is determined by the
// procDelay module parameter.
//
// The current performance model comes from the QueueBase C++ base class.
// If you need a more sophisticated performance model, you may change the
// module implementation (the IPv4 class), and: (1) override the startService()
// method which determines processing time for a packet, or (2) use a
// different base class.
//
// @see ~IPv4RoutingTable, ~IPv4ControlInfo, ~ARP
//
// @author Andras Varga
//
simple IPv4
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string arpModule;
        string icmpModule;
        double procDelay @unit("s") = default(0s);
        int timeToLive = default(32);
        int multicastTimeToLive = default(32);
        double fragmentTimeout @unit("s") = default(60s);
        bool forceBroadcast = default(false);
        bool useProxyARP = default(true);
        @display("i=block/routing");
        @signal[NF_IPv4_NEW_MULTICAST](type=IPv4Datagram);
        @signal[NF_IPv4_DATA_ON_NONRPF](type=IPv4Datagram);
        @signal[NF_IPv4_DATA_ON_RPF](type=IPv4Datagram);
        @signal[NF_IPv4_MDATA_REGISTER](type=IPv4Datagram);
    gates:
        input transportIn[] @labels(IPv4ControlInfo/down,TCPSegment,UDPPacket);
        output transportOut[] @labels(IPv4ControlInfo/up,TCPSegment,UDPPacket);
        input arpIn @labels(ARPPacket+Ieee802Ctrl);
        output arpOut @labels(ARPPacket+Ieee802Ctrl,IPv4Datagram+Ieee802Ctrl);
        input queueIn[] @labels(IPv4Datagram,ARPPacket,Ieee802Ctrl);
        output queueOut[] @labels(IPv4Datagram,ARPPacket,Ieee802Ctrl);
}
