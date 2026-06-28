//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.isis.routing;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;
import inet.node.isis.IsisRouter;

//
// A three-router IS-IS network: h1 -- r1 -- r2 -- r3 -- h3. IS-IS runs on the
// inter-router links only; the host subnets are advertised as IP reachability.
// The configurator assigns addresses and direct routes but NOT remote routes,
// so connectivity between h1 and h3 depends entirely on the routes computed and
// installed by IS-IS (Integrated IS-IS).
//
network IsisNetwork
{
    @display("bgb=720,160");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=360,30");
        }
        h1: StandardHost {
            @display("p=60,100");
        }
        r1: IsisRouter {
            @display("p=200,100");
        }
        r2: IsisRouter {
            @display("p=360,100");
        }
        r3: IsisRouter {
            @display("p=520,100");
        }
        h3: StandardHost {
            @display("p=660,100");
        }
    connections allowunconnected:
        h1.ethg++ <--> Eth100M <--> r1.ethg++;
        r1.ethg++ <--> Eth100M <--> r2.ethg++;
        r2.ethg++ <--> Eth100M <--> r3.ethg++;
        r3.ethg++ <--> Eth100M <--> h3.ethg++;
}
