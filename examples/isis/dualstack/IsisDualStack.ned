//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.isis.dualstack;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;
import inet.node.isis.IsisRouter;

//
// A dual-stack IS-IS network: hostA -- r1 -- r2 -- r3 -- hostB. The routers run
// single-topology Integrated IS-IS for BOTH IPv4 and IPv6 on the inter-router
// links; one set of LSPs carries both the IPv4 and the IPv6 reachability. The
// hosts ping each other over both address families, each succeeding only via
// the routes IS-IS computes.
//
network IsisDualStack
{
    @display("bgb=720,180");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=300,30");
        }
        configurator6: Ipv6NetworkConfigurator {
            @display("p=440,30");
        }
        hostA: StandardHost {
            @display("p=60,110");
        }
        r1: IsisRouter {
            @display("p=200,110");
        }
        r2: IsisRouter {
            @display("p=360,110");
        }
        r3: IsisRouter {
            @display("p=520,110");
        }
        hostB: StandardHost {
            @display("p=660,110");
        }
    connections allowunconnected:
        hostA.ethg++ <--> Eth100M <--> r1.ethg++;
        r1.ethg++ <--> Eth100M <--> r2.ethg++;
        r2.ethg++ <--> Eth100M <--> r3.ethg++;
        r3.ethg++ <--> Eth100M <--> hostB.ethg++;
}
