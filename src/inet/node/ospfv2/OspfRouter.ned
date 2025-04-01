//
// Copyright (C) 2005 Andras Babos
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.ospfv2;

import inet.node.inet.Router;

//
// A router that uses the Open Shortest Path First version 2 (OSPFv2) routing
// protocol. It extends the standard Router module with OSPF functionality
// enabled.
//
// OSPF is a link-state routing protocol that operates within a single Autonomous
// System (AS). It builds and maintains a link-state database whose information is
// used to calculate the shortest path to each destination using Dijkstra's algorithm.
//
// @see ~RipRouter, ~BgpRouter
//
module OspfRouter extends Router
{
    parameters:
        hasOspf = true;
}

