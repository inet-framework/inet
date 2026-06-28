//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.isis.failure;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;
import inet.node.isis.IsisRouter;

//
// h1 -- r1 -- r2 -- r3 -- h3, used to demonstrate the IS-IS LSP lifecycle:
// when r3 is shut down mid-run its adjacency is lost, the routers withdraw the
// routes that depended on it, and its now-unrefreshed LSP ages out and is
// purged from the databases.
//
network IsisFailure
{
    @display("bgb=720,160");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=300,30");
        }
        scenarioManager: ScenarioManager {
            @display("p=520,30");
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
