//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.isis.ipv6;

import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ipv6.StandardHost6;
import inet.node.isis.IsisRouter;

//
// A pure-IPv6 IS-IS network: hostA -- r1 -- r2 -- r3 -- hostB. The routers run
// IPv6-only Integrated IS-IS (RFC 5308) on the inter-router links; the host
// subnets are advertised as IPv6 reachability. Connectivity between the hosts
// depends entirely on the IPv6 routes IS-IS computes.
//
network IsisIpv6
{
    @display("bgb=720,160");
    submodules:
        configurator: Ipv6NetworkConfigurator {
            @display("p=360,30");
        }
        hostA: StandardHost6 {
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
        hostB: StandardHost6 {
            @display("p=660,100");
        }
    connections allowunconnected:
        hostA.ethg++ <--> Eth100M <--> r1.ethg++;
        r1.ethg++ <--> Eth100M <--> r2.ethg++;
        r2.ethg++ <--> Eth100M <--> r3.ethg++;
        r3.ethg++ <--> Eth100M <--> hostB.ethg++;
}
