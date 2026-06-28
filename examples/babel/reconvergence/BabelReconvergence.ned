//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.babel.reconvergence;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;

//
// A redundant Babel network over IPv6:
//
//   h1 --- r1 ===== r2 --- h2
//           \       /
//            \     /
//             \   /
//              r3
//
// h1 reaches h2 over the direct r1--r2 link. At t=30s the ScenarioManager
// shuts down r1's interface to r2; Babel must withdraw the failed route,
// request a fresh sequence number, and reconverge onto the r1--r3--r2 path.
//
network BabelReconvergence
{
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        scenarioManager: ScenarioManager {
            @display("p=100,50");
        }
        configurator: Ipv6NetworkConfigurator {
            @display("p=100,150");
        }
        h1: StandardHost {
            @display("p=100,300;i=device/laptop");
        }
        r1: Router {
            @display("p=250,300");
        }
        r2: Router {
            @display("p=500,300");
        }
        r3: Router {
            @display("p=375,420");
        }
        h2: StandardHost {
            @display("p=650,300;i=device/laptop");
        }
    connections:
        h1.ethg++ <--> C <--> r1.ethg++; // r1.eth[0]
        r1.ethg++ <--> C <--> r2.ethg++; // r1.eth[1] -- the link that is broken
        r1.ethg++ <--> C <--> r3.ethg++; // r1.eth[2]
        r3.ethg++ <--> C <--> r2.ethg++;
        r2.ethg++ <--> C <--> h2.ethg++;
}
