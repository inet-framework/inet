//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.isis.adjacency;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.isis.IsisRouter;

//
// Two IS-IS routers on a single Ethernet link, used to demonstrate Level-1
// adjacency formation. IS-IS PDUs ride directly on the link layer (IEEE 802.2
// LLC), not on IP.
//
network IsisAdjacency
{
    @display("bgb=400,200");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=200,40");
        }
        r1: IsisRouter {
            @display("p=120,120");
        }
        r2: IsisRouter {
            @display("p=280,120");
        }
    connections:
        r1.ethg++ <--> Eth100M <--> r2.ethg++;
}
