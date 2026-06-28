//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.babel.routing;

import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;

//
// Two hosts separated by a chain of three Babel routers over IPv6:
//
//   h1 --- r1 --- r2 --- r3 --- h2
//
// The configurator assigns addresses and host default routes but no transit
// routes between the routers; Babel must discover and install them so that h1
// can reach h2 across the network.
//
network BabelRouting
{
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        configurator: Ipv6NetworkConfigurator {
            @display("p=100,100");
        }
        h1: StandardHost {
            @display("p=100,250;i=device/laptop");
        }
        r1: Router {
            @display("p=250,250");
        }
        r2: Router {
            @display("p=400,250");
        }
        r3: Router {
            @display("p=550,250");
        }
        h2: StandardHost {
            @display("p=700,250;i=device/laptop");
        }
    connections:
        h1.ethg++ <--> C <--> r1.ethg++;
        r1.ethg++ <--> C <--> r2.ethg++;
        r2.ethg++ <--> C <--> r3.ethg++;
        r3.ethg++ <--> C <--> h2.ethg++;
}
