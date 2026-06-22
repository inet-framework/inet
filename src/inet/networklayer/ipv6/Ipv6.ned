//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.ipv6;

import inet.common.SimpleModule;

//
// Implements the IPv6 protocol.
//
// The IPv6 protocol header is represented by the ~Ipv6Header message class.
//
// <b>Interfacing with higher layer protocols</b>
//
// To send a packet over IPv6 from a higher layer protocol, the module should
// fill in an ~L3AddressReq object, attach it to the packet with Packet's
// `addTag()` method, then send the packet to the ~Ipv6 module.
//
// When ~Ipv6 sends up a packet to a higher layer protocol, it will also attach
// an ~L3AddressInd to the packet, with the source and destination IPv6 addresses
// of the ~Ipv6Header in which the packet arrived.
//
// ~Ipv6 can serve several higher-layer protocols. The higher layer protocols
// should call `registerProtocol` with their gate towards the IPv6 module,
// to fill up the protocol-to-gateindex map. When delivering packets to them,
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
// Routing protocol implementations can also query and manipulate the route table
// by calling ~Ipv6RoutingTable's methods in C++.
//
// <b>Netfilter hooks</b>
//
// Other modules can register INetfilter hooks on ~Ipv6 to inspect, modify, drop
// or queue datagrams at well-defined points (pre-routing, local-out,
// post-routing). Mobile IPv6 (~Mipv6) uses these hooks to steer home-address
// traffic onto tunnel interfaces and to insert the Type 2 Routing Header / Home
// Address Option on route-optimized traffic, so the IPv6 core itself needs no
// mobility-specific knowledge. IPv6-in-IPv6 tunnels (RFC 2473) are likewise plain
// ~Ipv6TunnelInterface network interfaces plus routes, not a special path in ~Ipv6.
//
// <b>Performance model, QoS</b>
//
// In the current form, ~Ipv6 contains a FIFO which queues up IPv6 frames;
// these are processed in order. The processing time is determined by the
// `procDelay` module parameter.
//
// @see ~Ipv6RoutingTable, ~Ipv6NeighbourDiscovery, ~Icmpv6
//
simple Ipv6 extends SimpleModule
{
    parameters:
        @class(Ipv6);
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string ipv6NeighbourDiscoveryModule;
        string icmpv6Module;
        double procDelay @unit(s) = default(0s);
        bool sendRedirects = default(true); // RFC 4861 Section 8.2: when a router forwards a packet back out the interface it arrived on, send the source an ICMPv6 Redirect. Disable on wireless multi-hop / ad-hoc networks (e.g. MANET routing), where the geographic next hop is not directly reachable by the source, so redirects are useless and trigger a storm.
        displayStringTextFormat = default("{ipv6StatusText}");
        @display("i=block/network2");
        @signal[packetDropped](type=inet::Packet);
        @signal[packetSentToUpper](type=inet::Packet);
        @signal[packetReceivedFromUpper](type=inet::Packet);
        @signal[packetSentToLower](type=inet::Packet);
        @signal[packetReceivedFromLower](type=inet::Packet);
        @signal[ipv6NewMulticast](type=inet::Ipv6Header);
        @signal[ipv6DataOnNonrpf](type=inet::Ipv6Header);
        @signal[ipv6DataOnRpf](type=inet::Ipv6Header);
        @signal[ipv6MdataRegister](type=inet::Packet);
    gates:
        input transportIn @labels(Ipv6ControlInfo/down,TcpHeader,UdpHeader);
        output transportOut @labels(Ipv6ControlInfo/up,TcpHeader,UdpHeader);
        input queueIn @labels(IPv6Datagram);
        output queueOut @labels(IPv6Datagram);
        input ndIn;
        output ndOut;
}

