//
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
// Implements the Address Resolution Protocol for IPv4 and IEEE 802 6-byte 
// MAC addresses.
//
// Expects IPRoutingDecision control info objects to be attached to packets
// coming from higher layers. \ARP packets are represented by the ARPPacket
// class. Datagrams to be sent and incoming \ARP packets are expected to
// on gate ipIn.
//
// \ARP resolution is used over <i>broadcast</i> interfaces only,
// that is, over interfaces that have the <tt>isBroadcast()</tt> flag set in 
// <tt>InterfaceEntry</tt> (see InterfaceTable). Since routing files 
// (<tt>.irt</tt> or <tt>.mrt</tt> files, given as parameter to RoutingTable)
// may modify the default interface configuration, you must take care that
// these files don't contain the word <tt>BROADCAST</tt> e.g. for PPP
// interfaces.
//
simple ARP
    parameters:
        retryTimeout: numeric, // number seconds ARP waits between retries to resolve an \IP address
        retryCount: numeric,   // number of times ARP will attempt to resolve an \IP address
        cacheTimeout: numeric, // number seconds unused entries in the cache will time out
        proxyARP: bool;        // sets proxy \ARP mode (replying to \ARP requests for the addresses for which a routing table entry exists)
    gates:
        in: ipIn;
        out: nicOut[];
endsimple
