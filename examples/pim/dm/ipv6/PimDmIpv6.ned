//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.pim.dm.ipv6;

import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.MulticastRouter;
import inet.node.ipv6.StandardHost6;

//
// An IPv6 multicast router running PIM (Dense Mode). It extends MulticastRouter
// (PIM + multicast forwarding already enabled) and runs the PIM data plane over
// IPv6.
//
// Both IP stacks are present: the IPv6 stack carries all data and PIM-DM
// traffic, while the IPv4 stack is kept only because the Pim compound always
// instantiates an (unused) PIM-SM submodule, and PIM-SM is IPv4-only and binds
// an IPv4 routing table at init. The IPv4 stack is otherwise left unconfigured
// (its node configurator is disabled in the ini), so it has no addresses and no
// effect on the simulation.
//
// pimDM and the splitter run with addressFamily="ipv6" and use the IPv6 routing
// table; pimSM keeps the ipv4 defaults from Pim.ned / Router.ned.
//
module MulticastRouter6 extends MulticastRouter
{
    parameters:
        hasIpv6 = default(true);
        hasIpv4 = default(true); // kept so the idle PIM-SM can bind an IPv4 routing table
        // PIM data plane is IPv6 (pimDM + splitter); pimSM stays ipv4.
        pim.pimDM.addressFamily = "ipv6";
        pim.pimSplitter.addressFamily = "ipv6";
        // pimDM uses the IPv6 routing table; pimSM keeps the node's ipv4 default.
        pim.pimDM.routingTableModule = "^.^.ipv6.routingTable";
}

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
