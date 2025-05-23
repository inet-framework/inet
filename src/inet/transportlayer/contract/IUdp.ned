//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.contract;

//
// UDP protocol interface.
//
// The UDP protocol header is represented by the class ~UdpHeader.
//
// <b>Communication with clients (applications)</b>
//
// The module can (should) be connected to several applications.
// For sending a UDP packet, the application should attach an ~UdpCommand
// object to the payload, and send it to ~Udp.
// ~Udp will also attach an ~UdpCommand object to any payload
// message it sends up to the application.
//
// For receiving UDP packets, the connected applications should first
// "bind" to the given UDP port. This can be done by sending an
// arbitrary message with message kind UDP_C_BIND and an ~UdpCommand
// attached with `srcPort` filled in.
//
// If there is only one app which doesn't bind to any port, it will
// receive all packets.
//
// <b>Communication with the IPv4 (IPv4/IPv6) layer</b>
//
// The ~Udp model relies on sending and receiving ~L3AddressReq/~L3AddressInd
// objects attached to packets.
//
// @see ~Udp, ~UdpHeader, ~UdpCommand, ~UdpCommandCode
//
moduleinterface IUdp
{
    parameters:
        @display("i=block/transport");

    gates:
        input appIn @labels(UdpCommand/down);
        input ipIn @labels(UdpHeader,Ipv4ControlInfo/up,Ipv6ControlInfo/up);
        output appOut @labels(UdpCommand/up);
        output ipOut @labels(UdpHeader,Ipv4ControlInfo/down,Ipv6ControlInfo/down);
}

