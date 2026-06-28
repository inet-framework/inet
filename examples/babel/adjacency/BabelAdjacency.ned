//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.babel.adjacency;

import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
import inet.node.inet.Router;
import ned.DatarateChannel;

//
// Three Babel routers in a line (r1 -- r2 -- r3) over IPv6. Demonstrates Babel
// neighbour discovery: the routers exchange Hello/IHU TLVs over the IPv6
// link-local multicast group and establish two-way adjacencies with finite
// link cost.
//
network BabelAdjacency
{
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        configurator: Ipv6FlatNetworkConfigurator {
            @display("p=100,100");
        }
        r1: Router {
            @display("p=250,200");
        }
        r2: Router {
            @display("p=400,200");
        }
        r3: Router {
            @display("p=550,200");
        }
    connections:
        r1.ethg++ <--> C <--> r2.ethg++;
        r2.ethg++ <--> C <--> r3.ethg++;
}
