//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.isis.lan;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;
import inet.node.isis.IsisRouter;

//
// Three IS-IS routers on a single shared Ethernet LAN (via a switch), each with
// a host on its own stub subnet. The routers run IS-IS only on their LAN-facing
// interface; they elect a Designated IS, which originates a pseudonode LSP that
// represents the LAN. Reachability between the stub subnets depends entirely on
// the routes IS-IS computes through that pseudonode.
//
network IsisLan
{
    @display("bgb=560,320");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=280,30");
        }
        switch: EthernetSwitch {
            @display("p=280,160");
        }
        r1: IsisRouter {
            @display("p=120,160");
        }
        r2: IsisRouter {
            @display("p=280,250");
        }
        r3: IsisRouter {
            @display("p=440,160");
        }
        h1: StandardHost {
            @display("p=120,280");
        }
        h2: StandardHost {
            @display("p=280,310");
        }
        h3: StandardHost {
            @display("p=440,280");
        }
    connections allowunconnected:
        h1.ethg++ <--> Eth100M <--> r1.ethg++;
        h2.ethg++ <--> Eth100M <--> r2.ethg++;
        h3.ethg++ <--> Eth100M <--> r3.ethg++;
        r1.ethg++ <--> Eth100M <--> switch.ethg++;
        r2.ethg++ <--> Eth100M <--> switch.ethg++;
        r3.ethg++ <--> Eth100M <--> switch.ethg++;
}
