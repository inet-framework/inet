//***************************************************************************
// file:        WiseRoute.ned
//
// author:      Damien Piguet, Jerome Rousselot
//
// copyright:   (C) 2007-2009 CSEM SA, Neuchatel, Switzerland.
//
// description: Implementation of the routing protocol of WiseStack.
//
// *              This program is free software; you can redistribute it
// *              and/or modify it under the terms of the GNU General Public
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later
// *              version
// *              For further information see file COPYING
// *              in the top level directory
// *
// * Funding: This work was partially financed by the European Commission under the  
// * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
// * (WASP) under contract IST-034963.
// ***************************************************************************
// * ported to Mixim 2.0.1 by Theodoros Kapourniotis
// * last modification: 06/02/11
// **************************************************************************/
package inet.networklayer.wiseroute;

import inet.networklayer.base.NetworkProtocolBase;
import inet.networklayer.contract.INetworkProtocol;

//
// Wiseroute is a simple loop-free routing algorithm that
// builds a routing tree from a central network point, designed
// for sensor networks and convergecast traffic.
//
// The sink (the device at the center of the network) broadcasts
// a route building message. Each network node that receives it
// selects the sink as parent in the routing tree, and rebroadcasts
// the route building message. This procedure maximizes the probability
// that all network nodes can join the network, and avoids loops.
// Parameter sinkAddress gives the sink network address,
// rssiThreshold is a threshold to avoid using bad links (with too low
// RSSI values) for routing, and routeFloodsInterval should be set to
// zero for all nodes except the sink. Each routeFloodsInterval, the
// sink restarts the tree building procedure. Set it to a large value
// if you do not want the tree to be rebuilt.
//
simple WiseRoute extends NetworkProtocolBase like INetworkProtocol
{
    parameters:
        int headerLength @unit(bit) = default(100b); // length of the network packet header (in bits)

        // debug switch
        bool debug = default(false);
        bool trace = default(false);
        bool useSimTracer = default(false);

        // sink node address
        string sinkAddress;
        // the sink directs the tree building procedure with periodic floods.
        // iterationDelay is the period between two floods.

        // RSSI threshold for route selection
        double rssiThreshold @unit(dBm) = default(-50 dBm);

        // If set to zero, this node does not initiates route tree building.
        // If set to a value larger than zero, this nodes periodically initiates route tree building.
        double routeFloodsInterval @unit(s) = default(0 s);

        string arpModule; // The path to the ARP module

        @display("i=block/fork");
        @class(WiseRoute);
}
