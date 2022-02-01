//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.dsdv;

import inet.node.inet.AdhocHost;
import inet.routing.dsdv.Dsdv;

//
// A DSDV router.
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

