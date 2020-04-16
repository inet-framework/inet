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
// The IPv6 protocol header is represented by the ~Ipv6Header message class.
//
// <b>Interfacing with higher layer protocols</b>
//
// To send a packet over IPv6 from a higher layer protocol, the module should
// fill in an ~L3AddressReq object, attach it to the packet with Packet's
// addTag() method, then send the packet to the ~Ipv6 module.
//
// When ~Ipv6 sends up a packet to a higher layer protocol, it will also attach
// an ~L3AddressInd to the packet, with the source and destination IPv6 addresses
// of the ~Ipv6Header in which the packet arrived.
//
// ~Ipv6 can serve several higher-layer protocols. The higher layer protocols
// should call registerProtocol with their gate towards the IPv6 module,
// for fill up the protocol-to-gateindex map. When delivering packets to them,
// the output gate is determined from the Protocol field in the IPv6 header.
//
// <b>Routing and interfacing with lower layers</b>
//
// The routing table is stored in the module ~Ipv6RoutingTable. When a datagram
// needs to be routed, Ipv6 queries ~Ipv6RoutingTable for the output interface
// (or "port") and next hop address of the packet. This is done by directly
// calling C++ methods of ~Ipv6RoutingTable. No message exchange with ~Ipv6RoutingTable
// takes place.
//
//#FIXME TBD describe operation
//
// Routing protocol implementations can also query and manipulate the route table
// by calling ~Ipv6RoutingTable's methods in C++.
//
// <b>Performance model, QoS</b>
//
// In the current form, ~Ipv6 contains a FIFO which queues up IPv6 frames;
// these are processed in order. The processing time is determined by the
// procDelay module parameter.
//
// @see ~Ipv6RoutingTable, ~Ipv6NeighbourDiscovery, ~Icmpv6
//
// @author Andras Varga
//
simple Ipv6
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string ipv6NeighbourDiscoveryModule;
        string icmpv6Module;
        string ipv6TunnelingModule;
        double procDelay @unit(s) = default(0s);
        @display("i=block/network2");
        @signal[packetDropped](type=cPacket);
        @signal[packetSentToUpper](type=inet::Packet);
    gates:
        input transportIn @labels(Ipv6ControlInfo/down,TcpHeader,UdpHeader);
        output transportOut @labels(Ipv6ControlInfo/up,TcpHeader,UdpHeader);
        input queueIn @labels(IPv6Datagram);
        output queueOut @labels(IPv6Datagram);
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

