//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.routing.pim;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.MulticastRouter;
import inet.node.inet.StandardHost;

//
// Network topology for comparing PIM-DM and PIM-SM multicast routing.
// The same topology is used with both dense mode and sparse mode
// configurations to highlight their different behaviors.
//
// Receiver1 is interested in the multicast group; Receiver2 is not.
// In PIM-SM, R2 acts as the Rendezvous Point.
//
network PimComparisonShowcase
{
    submodules:
        Source: StandardHost {
            @display("p=150,200");
        }
        R1: MulticastRouter {
            @display("p=300,200");
        }
        R2: MulticastRouter {
            @display("p=450,100");
        }
        R3: MulticastRouter {
            @display("p=600,100");
        }
        R4: MulticastRouter {
            @display("p=450,300");
        }
        Receiver1: StandardHost {
            @display("p=750,100");
        }
        Receiver2: StandardHost {
            @display("p=600,300");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,400;is=s");
        }
    connections allowunconnected:
        Source.ethg++ <--> Eth100M <--> R1.ethg++;
        R1.ethg++ <--> Eth100M <--> R2.ethg++;
        R1.ethg++ <--> Eth100M <--> R4.ethg++;
        R2.ethg++ <--> Eth100M <--> R3.ethg++;
        R4.ethg++ <--> Eth100M <--> R3.ethg++;
        R3.ethg++ <--> Eth100M <--> Receiver1.ethg++;
        R4.ethg++ <--> Eth100M <--> Receiver2.ethg++;
}
