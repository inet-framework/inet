//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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
// Handles local delivery of IP datagrams, the main task being defragmentation.
//
// Receive IP datagram for local delivery:
// - strip off IP header
// - buffer fragments for ip_fragmenttime
// - wait until all fragments of one fragment number are received
// - discard without notification if not all fragments arrive in ip_fragmenttime
//
// Once all fragments have arrived:
// - send Transport packet up to the transport layer
// - send ICMP packet to ICMP module
// - send IGMP group management packet to Multicast module
// - send tunneled IP datagram to PreRouting
//
// Author: Jochen Reber, date: 2.5.00, 9.5.00, 20.5.00, 9.6.00
// Redesign: Andras Varga, 02.2004
//
simple LocalDeliver
    parameters:
        procdelay: numeric const,
        qosBehaviorClass: string,
        fragmentTimeout : string;
    gates:
        in: in[];
        out: multicastOut;
        out: ICMPOut;
        out: preRoutingOut;
        out: transportOut[];
endsimple


