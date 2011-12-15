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

package inet.networklayer.arp;

//
// Implements the Address Resolution Protocol for IPv4 and IEEE 802 6-byte
// MAC addresses.
//
// Expects ~IPv4RoutingDecision control info objects to be attached to packets
// coming from higher layers. ARP packets are represented by the ~ARPPacket
// class. Datagrams to be sent and incoming ARP packets are expected to
// on gate ipIn.
//
// ARP resolution is used over <i>broadcast</i> interfaces only,
// that is, over interfaces that have the <tt>isBroadcast()</tt> flag set in
// <tt>InterfaceEntry</tt> (see ~InterfaceTable). Since routing files
// (<tt>.irt</tt> or <tt>.mrt</tt> files, given as parameter to ~RoutingTable)
// may modify the default interface configuration, you must take care that
// these files don't contain the word <tt>BROADCAST</tt> e.g. for PPP
// interfaces.
//
simple ARP
{
    parameters:
        double retryTimeout @unit("s") = default(1s); // number seconds ARP waits between retries to resolve an IPv4 address
        int retryCount = default(3);   // number of times ARP will attempt to resolve an IPv4 address
        double cacheTimeout @unit("s") = default(120s); // number seconds unused entries in the cache will time out
        bool proxyARP = default(true);        // sets proxy \ARP mode (replying to \ARP requests for the addresses for which a routing table entry exists)
        bool globalARP = default(false);
        @display("i=block/layer");
        @statistic[sentReq](title="ARP request sent";record=count,vector);
        @statistic[sentReplies](title="ARP replies sent";record=count,vector);
        @statistic[initiatedResolution](title="ARP initiated resolutions";record=count,vector);
        @statistic[failedResolution](title="ARP failed resolutions";record=count,vector);
    gates:
        input ipIn @labels(ARPPacket,IPv4Datagram);
        output nicOut[] @labels(ARPPacket);
}
