//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.pim.dm_ipv6;

import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ipv6.MulticastRouter6;
import inet.node.ipv6.StandardHost6;

//
// IPv6 PIM-DM flood-and-prune topology, modeled on the IPv4
// examples/pim/dm/basic network (pimDMFinal) but over IPv6. A source behind R1
// sends to an IPv6 multicast group; PIM-DM floods it over IPv6 and prunes the
// branches without listeners. Host2 (behind R2) is an MLD receiver of the
// group; Host3 (behind R3) joins later to trigger a Graft.
//
network PimDmIpv6
{
    submodules:
        R1: MulticastRouter6 {
            @display("p=425,190");
        }
        R2: MulticastRouter6 {
            @display("p=275,280");
        }
        R3: MulticastRouter6 {
            @display("p=575,280");
        }

        Source1: StandardHost6 {
            @display("p=425,100");
        }
        Host1: StandardHost6 {
            @display("p=200,370;i=device/pc");
        }
        Host2: StandardHost6 {
            @display("p=350,370;i=device/pc");
        }
        Host3: StandardHost6 {
            @display("p=650,370;i=device/pc");
        }
        configurator: Ipv6FlatNetworkConfigurator {
            @display("p=100,100;is=s");
        }
    connections allowunconnected:
        R1.ethg++ <--> Eth100M <--> R2.ethg++;
        R1.ethg++ <--> Eth100M <--> R3.ethg++;
        R1.ethg++ <--> Eth100M <--> Source1.ethg++;

        R2.ethg++ <--> Eth100M <--> Host1.ethg++;
        R2.ethg++ <--> Eth100M <--> Host2.ethg++;

        R3.ethg++ <--> Eth100M <--> Host3.ethg++;
}
