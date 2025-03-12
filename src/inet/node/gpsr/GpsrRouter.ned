//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.gpsr;

import inet.node.inet.AdhocHost;
import inet.routing.gpsr.Gpsr;

//
// A mobile node with the Greedy Perimeter Stateless Routing (GPSR) protocol. It
// extends AdhocHost by adding the GPSR routing protocol module.
//
// GPSR is a position-based routing protocol designed for wireless ad hoc networks.
// It uses the geographic position of nodes to make packet forwarding decisions.
// The protocol operates in two modes: greedy forwarding when possible, and perimeter
// routing when necessary to navigate around voids (areas with no nodes).
//
module GpsrRouter extends AdhocHost
{
    submodules:
        gpsr: Gpsr {
            parameters:
                @display("p=825,226");
        }
    connections:
        gpsr.ipOut --> tn.in++;
        gpsr.ipIn <-- tn.out++;
}

