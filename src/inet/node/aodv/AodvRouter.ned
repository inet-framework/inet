//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.aodv;

import inet.node.inet.AdhocHost;
import inet.routing.aodv.Aodv;

//
// A mobile node with the Ad hoc On-Demand Distance Vector (AODV)
// routing protocol. Extends AdhocHost by adding the AODV routing protocol module.
//
// AODV is a reactive routing protocol designed for mobile ad hoc networks (MANETs).
// Each node maintains its own routing table and discovers routes on-demand when
// communication is needed, rather than proactively maintaining routes to all destinations.
//
// Key features:
// - On-demand route discovery using Route Request (RREQ) and Route Reply (RREP) messages
// - Route maintenance through Route Error (RERR) messages when links break
// - Sequence numbers to ensure loop-free routes and route freshness
// - Hop-by-hop routing where each node maintains only the next hop information
// - Inherits wireless capabilities and mobility support from AdhocHost
//
// The AODV v.2 (known as Dymo) is also implemented in INET as ~Dymo and ~DymoRouter.
//
// @see ~AdhocHost, ~DymoRouter, ~ManetRouter
//
module AodvRouter extends AdhocHost
{
    submodules:
        aodv: Aodv {
            @display("p=825,226");
        }
    connections:
        aodv.socketOut --> at.in++;
        aodv.socketIn <-- at.out++;
}

