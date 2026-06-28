//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.babel.dualstack;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;

//
// Dual-stack Babel: the same chain of three routers carries both IPv4 and IPv6,
// and h1 reaches h2 over both protocols using routes that Babel discovers.
//
//   h1 --- r1 --- r2 --- r3 --- h2
//
network BabelDualStack
{
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,80");
        }
        configurator6: Ipv6NetworkConfigurator {
            @display("p=100,160");
        }
        h1: StandardHost {
            @display("p=100,300;i=device/laptop");
        }
        r1: Router {
            @display("p=250,300");
        }
        r2: Router {
            @display("p=400,300");
        }
        r3: Router {
            @display("p=550,300");
        }
        h2: StandardHost {
            @display("p=700,300;i=device/laptop");
        }
    connections:
        h1.ethg++ <--> C <--> r1.ethg++;
        r1.ethg++ <--> C <--> r2.ethg++;
        r2.ethg++ <--> C <--> r3.ethg++;
        r3.ethg++ <--> C <--> h2.ethg++;
}
