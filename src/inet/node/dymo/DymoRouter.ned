//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.dymo;

import inet.node.inet.AdhocHost;
import inet.routing.dymo.Dymo;

//
// A mobile node with the Dynamic MANET On-demand (DYMO) routing protocol.
// Extends AdhocHost by adding the DYMO routing protocol module.
//
// DYMO is a reactive routing protocol designed for mobile ad hoc networks (MANETs).
// It discovers routes on-demand when needed for communication, rather than proactively
// maintaining routes to all destinations. This approach is efficient for networks with
// mobile nodes where topology changes frequently.
//
// @see ~AodvRouter
//
module DymoRouter extends AdhocHost
{
    submodules:
        dymo: Dymo {
            @display("p=825,226");
        }
    connections:
        dymo.ipOut --> tn.in++;
        dymo.ipIn <-- tn.out++;
}

