//
//   Copyright (C) 2009 by Alfonso Ariza
//   Malaga University
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.routing.extras;

import inet.networklayer.IManetRouting;
import inet.routing.extras.base.BaseRouting;


//
// Ad hoc On-Demand Distance Vector (AODV) Routing (rfc3561)
//
// Implementation is based on rfc3561 draft 13 and implements
// all features defined in the rfc except noted below:
//
// NOTE:
//  - The initialization of the routing component is done in stage=4
//  - Message lengths are calculated assuming IPv4 addresses
//
simple AODVUU extends BaseRouting like IManetRouting
{
    parameters:
        string interfaceTableModule;
        string routingTableModule;
        string icmpModule = default("^.networkLayer.icmp");
        string arpModule = default("^.networkLayer.arp");
        @class(AODVUU);
        @reactive;                             // IP module will send control messages when no route is present to the destination
        bool log_to_file = default(false);     // dump AODV log files (non RFC parameter)
        bool hello_jittering = default(true);  // add a +-50ms jitter to the hello interval (non RFC parameter)
        bool optimized_hellos = default(true); // do not send hello messages if there are no active routes (non RFC parameter)
        bool expanding_ring_search = default(true);  // use expanding ring search (section 6.4)
        bool local_repair = default(true);    // allow the routes to be locally repaired if a node detects link failure (section 6.12)
        bool rreq_gratuitous = default(true); // force gratuitous flag on all RREQs (section 6.6.3)
        int rt_log_interval @unit(ms) = default(0ms);     // periodically log routing table to routing table logfile, value is the interval in msecs (0 = off)
        int unidir_hack = default(0);         // detect and avoid unidirectional links (boolean) (section 6.8)
        int receive_n_hellos = default(1);    // receive N HELLOs before treating as neighbor (non RFC parameter)
        int wait_on_reboot = default(1);      // wait after reboot and gather routes before sending out routing info (section 6.13)
        int ratelimit = default(1);            // do not send out more than 10 RERR or RREQ (boolean) (section 6.3 abd 6.11)
        int active_timeout @unit(ms) = default(3000ms);    // active route timeout in (ms) (section 6.2 and 6.7)
        bool llfeedback = default(false);      // allow layer2 link-detection instead of sending hello messages (section 6.10)
        int internet_gw_mode = default(0);    // NOT SUPPORTED  (boolean) (run this node as an internet gateway (non RFC parameter))
        string internet_gw_address = default("0.0.0.0"); // NOT SUPPORTED
        bool targetOnlyRreq = default(false);
        bool EqualDelay = default(true);       // add a jitter to broadcast operations (RREQ, HELLO broadcast) (non RFC - omnetpp specific)
        volatile double broadcastDelay @unit("s") = default(uniform(0s,0.005s));  // the delay added to broadcast operations if EqualDelay is set (used to model processing time)
        volatile double unicastDelay @unit("s") = default(0s);  // a delay added to unicast messaged (i.e. data packet forwarding) (used to model processing time)
        bool UseIndex = default(false);         // use the interface index instead the ip to identify the interface
        bool useHover = default(false);
        int costStatic = default(1);
        int costMobile = default(4);
        bool isRoot = default(false); // Proactive RREQ
        int proactiveRreqTimeout @unit(ms) = default(5000ms);// 5 seconds
        int proactiveLifeTime @unit(ms) = default(0ms); // if zero lifetime is PATH_DISCOVERY_TIME - 2 * rreq_new_hcnt * NODE_TRAVERSAL_TIME;
        int startRreqProactive @unit(ms) = default(uniform(1000ms,2000ms));
        bool propagateProactive = default(true); // Proactive feedback
        bool checkNextHop = default(false);
        bool PublicRoutingTables = default(false);
    gates:
        input from_ip;
        output to_ip;
}

