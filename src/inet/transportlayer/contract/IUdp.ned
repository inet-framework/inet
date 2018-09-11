//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2005 Andras Varga
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

package inet.transportlayer.contract;

//
// UDP protocol interface.
//
// The UDP protocol header is represented by the class ~UdpHeader.
//
// <b>Communication with clients (applications)</b>
//
// The module can (should) be connected to several applications.
// For sending an Udp packet, the application should attach an ~UdpControlInfo
// object to the payload, and send it to ~Udp.
// ~Udp will also attach an ~UdpControlInfo object to any payload
// message in sends up to the application.
//
// For receiving UDP packets, the connected applications should first
// "bind" to the given UDP port. This can be done by sending an
// arbitrary message with message kind UDP_C_BIND and an ~UdpControlInfo
// attached with srcPort filled in.
//
// If there is only one app which doesn't bind to any port, it will
// receive all packets.
//
// <b>Communication with the IPv4 (IPv4/IPv6) layer</b>
//
// The ~Udp model relies on sending and receiving ~L3AddressReq/~L3AddressInd
// objects attached to packets.
//
// @see ~Udp, ~UdpHeader, ~UdpControlInfo, ~UdpCommandCode
//
moduleinterface IUdp
{
    parameters:
        @display("i=block/transport");

    gates:
        input appIn @labels(UdpControlInfo/down);
        input ipIn @labels(UdpHeader,Ipv4ControlInfo/up,Ipv6ControlInfo/up);
        output appOut @labels(UdpControlInfo/up);
        output ipOut @labels(UdpHeader,Ipv4ControlInfo/down,Ipv6ControlInfo/down);
}

