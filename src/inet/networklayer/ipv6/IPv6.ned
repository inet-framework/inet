//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.


package inet.networklayer.ipv6;

//
// Implements the IPv6 protocol.
//
// An overview of the IPv6 implementation in the INET Framework is
// provided <a href="ipv6overview.html">here</a>.
//
// The IPv6 protocol header is represented by the ~IPv6Datagram message class.
//
// <b>Interfacing with higher layer protocols</b>
//
// To send a packet over IPv6 from a higher layer protocol, the module should
// fill in an ~IPv6ControlInfo object, attach it to the packet with cMessage's
// setControlInfo() method, the send it to the ~IPv6 module.
//
// When ~IPv6 sends up a packet to a higher layer protocol, it will also attach
// an ~IPv6ControlInfo to the packet, with the source and destination IPv6 address,
// etc. of the IPv6 datagram in which the packet arrived.
//
// ~IPv6 can serve several higher-layer protocols. The higher layer protocols should
// send one or more RegisterProtocol message (a simple cMessage with
// ~IPRegisterProtocolCommand controlinfo and kind=IP_C_REGISTER_PROTOCOL)
// to IPv4 module, for fill up the protocol-to-gateindex map.
// When delivering packets to them, the output gate is determined from the Protocol
// field in the IPv6 header.
//
// <b>Routing and interfacing with lower layers</b>
//
// The routing table is stored in the module ~IPv6RoutingTable. When a datagram
// needs to be routed, IPv6 queries ~IPv6RoutingTable for the output interface
// (or "port") and next hop address of the packet. This is done by directly
// calling C++ methods of ~IPv6RoutingTable. No message exchange with ~IPv6RoutingTable
// takes place.
//
//#FIXME TBD describe operation
//
// Routing protocol implementations can also query and manipulate the route table
// by calling ~IPv6RoutingTable's methods in C++.
//
// <b>Performance model, QoS</b>
//
// In the current form, ~IPv6 contains a FIFO which queues up IPv6 datagrams;
// datagrams are processed in order. The processing time is determined by the
// procDelay module parameter.
//
// @see ~IPv6RoutingTable, ~IPv6ControlInfo, ~IPv6NeighbourDiscovery, ~ICMPv6
//
// @author Andras Varga
//
simple IPv6
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string ipv6NeighbourDiscoveryModule;
        string icmpv6Module;
        string ipv6TunnelingModule;
        double procDelay @unit("s") = default(0s);
        @display("i=block/network2");
    gates:
        input transportIn[] @labels(IPv6ControlInfo/down,TCPSegment,UDPPacket);
        output transportOut[] @labels(IPv6ControlInfo/up,TCPSegment,UDPPacket);
        input queueIn[] @labels(IPv6Datagram);
        output queueOut[] @labels(IPv6Datagram);
        input icmpIn;
        output icmpOut;
        input ndIn;
        output ndOut;
        // tunneling gates - CB
        input upperTunnelingIn;
        output upperTunnelingOut;
        input lowerTunnelingIn;
        output lowerTunnelingOut;
        //the following gates are added by Zarrar Yousaf on 19.06.07
        input xMIPv6In;
        output xMIPv6Out;
}

