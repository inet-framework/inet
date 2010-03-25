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



package inet.transport.udp;

//
// UDP protocol implementation, for IPv4 and IPv6.
//
// The \UDP protocol header is represented by the class UDPPacket.
//
// <b>Communication with clients (applications)</b>
//
// The module can (should) be connected to several applications.
// For sending an \UDP packet, the application should attach an UDPControlInfo
// object to the payload, and send it to UDP.
// UDP will also attach an UDPControlInfo object to any payload
// message in sends up to the application.
//
// For receiving \UDP packets, the connected applications should first
// "bind" to the given \UDP port. This can be done by sending an
// arbitrary message with message kind UDP_C_BIND and an UDPControlInfo
// attached with srcPort filled in.
//
// If there is only one app which doesn't bind to any port, it will
// receive all packets.
//
// <b>Communication with the \IP (IPv4/IPv6) layer</b>
//
// The UDP model relies on sending and receiving IPControlInfo/IPv6ControlInfo
// objects attached to UDPPacket objects as control info
// (see cMessage::setControlInfo()).
//
// @see UDPPacket, UDPControlInfo, UDPCommandCode
//
simple UDP
{
    parameters:
        @display("i=block/transport");
    gates:
        input appIn[] @labels(UDPControlInfo/down);
        input ipIn @labels(UDPPacket,IPControlInfo/up);
        input ipv6In @labels(UDPPacket,IPv6ControlInfo/up);
        output appOut[] @labels(UDPControlInfo/up);
        output ipOut @labels(UDPPacket,IPControlInfo/down);
        output ipv6Out @labels(UDPPacket,IPv6ControlInfo/down);
}


