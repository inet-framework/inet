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

import inet.networklayer.IManetRouting;


//
// Ad hoc Dynamic Source Routing Routing (rfc4728)
//
// Implementation is based on rfc4728 draft 10 and implements
// all features defined in the rfc except noted below:
//
// NOTE:
//  - The initialization of the routing component is done in stage=4
//  - Message lengths are calculated assuming IPv4 addresses
//  - supports only a single wireless interface (radio) per host
//
simple DSRUU like IManetRouting
{
    parameters:
        @reactive;                             // IP module will send control messages when no route is present to the destination
        bool PrintDebug = default(false);  // print protocol depcific debugging information // (non RFC parameter)
        bool FlushLinkCache = default(true); // ??
        bool PromiscOperation = default(false);  // promiscuous mode is used to discover neighborhood nodes
        bool UseNetworkLayerAck = default(true); // use link layer level acknoledgements (section 8.3.1)
        int BroadcastJitter = default(0); // (ms) re-broadcast a rout request with an additional jitter (section 8.2.2). If 0 the code will use unicastDelay and broadcastDelay
        int RouteCacheTimeout = default(300); // (s) timeout for etries in the route cache (section 4.1)
        int SendBufferTimeout = default(30); // (s) how long packets without routes should be kept in the buffer
        int SendBufferSize = default(-1); // size of send buffer. -1 means default (which is 100)
        int RequestTableSize = default(-1);  // size of route request table. -1 means default (which is 64) (section 4.3)
        int RequestTableIds = default(-1);  // the number of Identification values to retain in each Route Request Table entry. (default is 16) (section 4.3)
        int MaxRequestRexmt = default(16);  // the maximum number fo retransmissions
        int MaxRequestPeriod = default(10); // (s) request retransmission preriod (section 8.2)
        int RequestPeriod = default(500); // (ms) starting period for route discoveries (section 8.2)
        int NonpropRequestTimeout = default(30); // (ms) ???
        int RexmtBufferSize = default(-1);   // size of retransmit buffer (in packets) (default 100)
        int MaintHoldoffTime = default(250); // (ms) do not confirm the reachability if info is not older than this time (section 8.3)
        int MaxMaintRexmt = default(2); // consider the link broken if no response is received from neighbor after trying this many (section 8.3)
        bool TryPassiveAcks = default(true); // try to request passive ack and switch to link layer ack only after passive ack failed
        int PassiveAckTimeout = default(100); // (ms) how often retransmit the packet if no passive ack was received
        int GratReplyHoldOff = default(1);    // (s) Life-time of the entry in the Gratuitous Route Reply Table (section 8.1.5)
        int MAX_SALVAGE_COUNT = default(15);  // section (8.2.3)
        int LifoSize = default(20); // ???
        int RREQMaxVisit = default(1);   // The node can be visit multiples times in the RREQ phase Gerla article
        bool RREPDestinationOnly = default(false); // only the destination send the RREP
        bool ETX_Active = default(false);  // activate ETX metrics
        double ETXHelloInterval = default(1); // how often to send out hello messages
        double ETXWindowNumHello = default(10); // number of hello transmissions
        int ETXRetryBeforeFail = default(-1); // if !=-1 use the etx how hello to detect break links,
                         // if don't receive consecutive ETXRetryBeforeFail number detect link break
        bool PathCache = default(true); // (non RFC parameter)
        bool RetryPacket = default(false); // (non RFC parameter)
        volatile double broadcastDelay @unit("s") = default(uniform(0s,0.005s));  // the delay added to broadcast operations if EqualDelay is set (used to model processing time)
        volatile double unicastDelay @unit("s") = default(0s);  // a delay added to unicast messaged (i.e. data packet forwarding) (used to model processing time)
        bool manetPurgeRoutingTables = default(true);
    gates:
        input from_ip;
        output to_ip;
}

