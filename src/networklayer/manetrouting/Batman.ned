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
simple Batman extends BaseRouting like IManetRouting
{
    parameters:
        @class(Batman);
        int debugLevel = default(1);
        double purgeTimeout @unit(s) = default(200s);
        double originatorInterval @unit(s) = default(1s);
        string preferedGateWay = default("0.0.0.0");
        int routingClass = default(0);
        bool agregationEnable = default(true);
        int GWClass_download_speed = default(0);
        int GWClass_upload_speed = default(0);
        volatile double jitter @unit(s) = default(uniform (0s,0.1s));
        volatile double jitter2 @unit(s) = default(uniform (-0.1s,0.1s));
        double MAX_AGGREGATION_MS @unit(s) = default(0.1s);
        int MAX_AGGREGATION_BYTES @unit(B) = default(512B);
        double desynchronized @unit(s) = default(0s);
        volatile double broadcastDelay @unit("s") = default(uniform(0s,0.005s));  // the delay added to broadcast operations if EqualDelay is set (used to model processing time)
        bool PublicRoutingTables = default(false);
    gates:
        input from_ip;
        output to_ip;
}



