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

import inet.networklayer.contract.IManetRouting;
import inet.routing.extras.base.BaseRouting;


//
// Optimized Link State Routing Protocol with the ETX and ML (Link Quality) extension
//
// Implements core functionality as described in rfc3626 chapter 2.1
// as well as the ETX and ML extensions (Auxiliary functions are NOT supported):
// Supports optional Link Layer feedback (see the use_mac parameter)
// and optional Fish eye (see the fish_eye parameter) extension.
//
// NOTE:
//  - The initialization of the routing component is done in stage=4
//  - hello, tc, mid intervals accept INTEGER values only
//  - IPv4 address size is assumed in the messages. If you need IPv6, recompile with -DOLSR_IPv6 added to CFLAGS
//  - Multiple interfaces per OLSR node is supported only with adding -DMULTIPLE_IFACES_SUPPORT to CFLAGS
//
// see: <a>http://www.inf.ufrgs.br/~~wlccordeiro/resources/olsr/README.html</a>
//
simple OLSR_ETX extends BaseRouting like IManetRouting
{
    parameters:
        @class(OLSR_ETX);
        string interfaceTableModule;
        string routingTableModule;
        string icmpModule = default("^.networkLayer.icmp");
        string arpModule = default("^.networkLayer.arp");
        int Willingness = default(3); // (0..7) The node's willingness to carry traffic on behalf of other nodes. 0:NEVER .. 7:ALWAYS (section 18.8)
        double Hello_ival @unit(s) = default(2s); // (s) HELLO messages' emission interval. (section 6.2)
        double Tc_ival @unit(s) = default(5s); // (s) Topology Control (TC) messages' emission interval. (section 9.3)
        double Mid_ival @unit(s) = default(5s); // (s) MID (multiple interface declaration) messages' emission interval. (section 5.2) (has effect only if compiled with multiple interface support)
        int use_mac = default(0); // Determines if layer 2 notifications are enabled or not (chapter 13)
        bool UseIndex = default(false); // use the interface index instead the ip to identify the interface (EXPERIMENTAL)
        bool reduceFuncionality = default(false);
        int Mpr_algorithm = default(1); // Indicate which MPR selection algorithm will be used (1:DEFAULT 2:R1 3:R2 4:QOLSR 5:OLSRD)
        int routing_algorithm = default(2); // Determine which routing algorithm is to be used (1:DEFAULT 2:DIJKSTRA)
        int Link_quality = default(2); // Determines which heuristic (1:NONE 2:ETX 3:ML) should be used for link quality computation
        bool Fish_eye = default(false);  // Determine whether fish eye extension should be used
        int Tc_redundancy @enum(0,1,2,3) = default(1); // Determine the redundancy level of TC messages (section 15.1)
        //  OLSR_ETX_TC_REDUNDANCY_MPR_SEL_SET               0
        // publish nodes in mpr sel set plus nodes in mpr set (RFC 3626)
        // OLSR_ETX_TC_REDUNDANCY_MPR_SEL_SET_PLUS_MPR_SET  1
        // publish full neighbor link set (RFC 3626)
        // OLSR_ETX_TC_REDUNDANCY_FULL                      2
        // non-OLSR standard: publish mpr set only
        // OLSR_ETX_TC_REDUNDANCY_MPR_SET                   3
        bool Link_delay = default(true);  // Determine whether the link delay extension should be enabled
        double C_alpha = default(0.6);  // Factor that will be used to smooth link delays (0 .. 1)
        ///
        /// \brief Period at which a node must cite every link and every neighbor.
        ///
        /// We only use this value in order to define OLSR_NEIGHB_HOLD_TIME.
        ///
        double OLSR_REFRESH_INTERVAL @unit("s") = default(2s);
        bool PublicRoutingTables = default(false);

        bool optimizedMid = default(false);// only nodes with more that a interface sends mid messages
        bool DelOnlyRtEntriesInrtable_ = default(false); // delete only ip entries that are in the interal routing table
    gates:
        input from_ip;
        output to_ip;
}

