//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.dymo;

import inet.node.inet.AdhocHost;
import inet.routing.dymo.Dymo;

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

