//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.ipv6.ipv6configurator;

import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.ipv6.Router6;
import inet.node.ipv6.StandardHost6;
import ned.DatarateChannel;

//
// Example network demonstrating the Ipv6NetworkConfigurator module.
//
// The network consists of two LANs connected by a backbone of two routers:
//
//    LAN1: host1, host2 --- router1
//    LAN2: host3, host4 --- router2
//    Backbone: router1 --- router2
//
// The configurator assigns hierarchical /64 prefixes from the documentation
// address space (2001:db8::/32), computes shortest-path static routes, and
// sets up AdvPrefixes on router interfaces.
//
network Ipv6ConfiguratorNetwork
{
    types:
        channel ethernet extends DatarateChannel
        {
            datarate = 100Mbps;
            delay = 0.1us;
        }
        channel backbone extends DatarateChannel
        {
            datarate = 1Gbps;
            delay = 1ms;
        }
    submodules:
        configurator: Ipv6NetworkConfigurator {
            parameters:
                @display("p=100,50;is=s");
        }
        // LAN 1
        host1: StandardHost6 {
            parameters:
                @display("p=150,300;i=device/pc");
        }
        host2: StandardHost6 {
            parameters:
                @display("p=350,300;i=device/pc");
        }
        router1: Router6 {
            parameters:
                @display("p=250,200");
        }
        // LAN 2
        host3: StandardHost6 {
            parameters:
                @display("p=550,300;i=device/pc");
        }
        host4: StandardHost6 {
            parameters:
                @display("p=750,300;i=device/pc");
        }
        router2: Router6 {
            parameters:
                @display("p=650,200");
        }
    connections:
        // LAN 1
        host1.ethg++ <--> ethernet <--> router1.ethg++;
        host2.ethg++ <--> ethernet <--> router1.ethg++;
        // LAN 2
        host3.ethg++ <--> ethernet <--> router2.ethg++;
        host4.ethg++ <--> ethernet <--> router2.ethg++;
        // Backbone
        router1.ethg++ <--> backbone <--> router2.ethg++;
}
