//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//
// Examines the packet's destination address and decides where it should
// be sent. The decision is stored in the IPRoutingDecision control info
// attached to the packet. The actual routing decision is done by
// invoking functions in the RoutingTable module.
//
// Detailed operation:
// - receive correct IPDatagram, then:
// - if source routing option is on, use next source addr. as dest. addr.
// - map \IP address on output port, use static routing table
// - if destination address is not in routing table,
//   throw datagram away and notify ICMP
// - process record route and timestamp options, if applicable
// - send to IPLocalDeliver if destination address is 127.0.0.1 (local
//   loopback) or the address of one of the network interfaces
// - send datagram with multicast address to IPMulticast
// - otherwise, send to IPFragmentation
//
// @see RoutingTable
//
// Author: Jochen Reber, date: 2.5.00, 9.5.00, 20.5.00, 9.6.00
// Redesign: Andras Varga, 02.2004
//
simple IPRouting
{
    parameters:
        double procDelay;
        string qosBehaviorClass;
        @display("i=block/routing");
    gates:
        input in[];
        output localOut;
        output fragmentationOut;
        output multicastOut;
}


