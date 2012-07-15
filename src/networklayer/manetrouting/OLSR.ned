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

package inet.networklayer.manetrouting;

import inet.networklayer.manetrouting.base.BaseRouting;
import inet.networklayer.IManetRouting;


//
// Optimized Link State Routing Protocol 
//
// Implements core functionality as described in rfc3626 chapter 2.1.
// Supports optional Link Layer feedback (see the use_mac parameter).
// Auxiliary functions are NOT supported.
//
// NOTE: 
//  - The initialization of the routing component is done in stage=4
//  - hello, tc, mid intervals accept INTEGER values only
//  - IPv4 addresses are assumed in the messages. If you need IPv6 recompile with -DOLSR_IPv6 added to CFLAGS 
//  - Multiple interfaces per OLSR node is supported only with adding -DMULTIPLE_IFACES_SUPPORT to CFLAGS
// 
simple OLSR extends BaseRouting like IManetRouting
{
    parameters:
        @class(OLSR);
        int Willingness = default(3); // (0..7) The node's willingness to carry traffic on behalf of other nodes. 0:NEVER .. 7:ALWAYS (section 18.8)
        int Hello_ival = default(2); // (s) HELLO messages' emission interval. (section 6.2)
        int Tc_ival = default(5); // (s) Topology Control (TC) messages' emission interval. (section 9.3)
        int Mid_ival = default(5); // (s) MID (multiple interface declaration) messages' emission interval. (section 5.2) (has effect only if compiled with multiple interface support)
        int use_mac = default(0); // Determines if layer 2 notifications are enabled or not (chapter 13)
        bool UseIndex = default(false); // use the interface index instead the ip to identify the interface (EXPERIMENTAL)
        bool reduceFuncionality = default(false);
        // OLSR timers
        double OLSR_HELLO_INTERVAL @unit("s")=default(2s);

	/// TC messages emission interval.
	    double OLSR_TC_INTERVAL @unit("s")=default(5s);

	/// MID messages emission interval.
	    double OLSR_MID_INTERVAL @unit("s")=default(5s);//   OLSR_TC_INTERVAL

	///
	/// \brief Period at which a node must cite every link and every neighbor.
	///
	/// We only use this value in order to define OLSR_NEIGHB_HOLD_TIME.
	///
	    double OLSR_REFRESH_INTERVAL @unit("s")=default(2s);
	    bool PublicRoutingTables = default(false);
	    
	    bool optimizedMid = default(false);// only nodes with more that a interface sends mid messages 

    gates:
        input from_ip;
        output to_ip;
}



