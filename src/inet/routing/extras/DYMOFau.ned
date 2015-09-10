//
//  Copyright (C) 2005 Mohamed Louizi
//  Copyright (C) 2006,2007 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package inet.routing.extras;

import inet.networklayer.contract.IManetRouting;
import inet.routing.extras.base.BaseRouting;


//
// Dynamic MANET On-demand (DYMO) Routing
//
// Implements draft-ietf-manet-dymo-10.
//
// NOTE:
//  - The initialization of the routing component is done in stage=4
//  - This implementation is a bit outdated. As of 2010-09-16 the
//    current draft number is 21
//
simple DYMOFau extends BaseRouting like IManetRouting
{
    parameters:
        @class(DYMOFau);
        @reactive;                             // IP module will send control messages when no route is present to the destination

        string interfaceTableModule;
        string routingTableModule;
        string icmpModule = default("^.networkLayer.icmp");
        string arpModule = default("^.networkLayer.arp");
        int RESPONSIBLE_ADDRESSES_PREFIX = default(-1); // netmask of network this DYMO router is responsible for, -1 for self only
        //        string DYMO_INTERFACES = default("wlan"); // list of interfaces to run DYMO on, separated by a single space character
        //        string AUTOASSIGN_ADDRESS_BASE = default("10.0.0.0"); // start of address range from which to automatically assign an address to the DYMO_INTERFACES
        double ROUTE_AGE_MIN_TIMEOUT @unit("s") = default(1s);
        double ROUTE_AGE_MAX_TIMEOUT @unit("s") = default(60s);
        double ROUTE_NEW_TIMEOUT @unit("s") = default(5s);
        double ROUTE_USED_TIMEOUT @unit("s") = default(5s);
        double ROUTE_DELETE_TIMEOUT @unit("s") = default(10s);
        int MIN_HOPLIMIT = default(5); // RREQs are first tried with this MsgHdr.HopLimit
        int MAX_HOPLIMIT = default(10); // MsgHdr.HopLimit for last RREQ, as well as other DYMO control messages
        double RREQ_RATE_LIMIT = default(10); // maximum average RREQs per second (token bucket)
        int RREQ_BURST_LIMIT = default(3); // maximum RREQs in a burst (token bucket)
        double RREQ_WAIT_TIME @unit("s") = default(2s);  // how log to wait for a Route Request
        int RREQ_TRIES = default(3); // how many times to retry a routing request before declaring the node unreachable
        int BUFFER_SIZE_PACKETS = default(50); // maximum queue size in packets, -1 for no limit
        int BUFFER_SIZE_BYTES @unit("B") = default(75000B); // maximum queue size in bytes, -1 for no limit
        bool PublicRoutingTables = default(false);
    gates:
        input from_ip;
        output to_ip;
}

