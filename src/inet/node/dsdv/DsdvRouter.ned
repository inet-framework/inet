//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.dsdv;

import inet.node.inet.AdhocHost;
import inet.routing.dsdv.Dsdv;

//
// A mobile node with the Destination-Sequenced Distance Vector
// (DSDV) routing protocol. Extends AdhocHost by adding the DSDV routing protocol module.
//
// DSDV is a proactive, table-driven routing protocol designed for mobile ad hoc networks.
// Each node maintains a routing table with entries for all possible destinations in the
// network, along with the next hop and distance (number of hops) to reach them.
//
// @see ~AodvRouter
//
module DsdvRouter extends AdhocHost
{
    submodules:
        dsdv: Dsdv {
            @display("p=825,226");
        }
    connections:
        dsdv.ipOut --> tn.in++;
        dsdv.ipIn <-- tn.out++;
}

