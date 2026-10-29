//
// Protocol Test Framework for INET -- a shared network: a host, a tap, a router and a host.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocoltest;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;

//
// host1 -- tap -- router -- host2. The tap sees the frames of host1 before the router, so a
// rule can make the router answer with a real ICMP error: a TTL of 1 gives Time Exceeded.
// The link from the router to host2 is the second one, router.eth[1]; the ini can give it a
// lower rate, a smaller MTU (*.router.eth[1].mtu) and another queue
// (*.router.eth[1].queue.typename), for path MTU discovery and congestion. The node types
// and the configurator are parameters, as in TapPairNetwork; the router is "Router6" for
// IPv6.
//
network TapRouterNetwork extends WiredNetworkBase
{
    parameters:
        double linkDatarate @unit(bps) = default(100Mbps);       // host1 -- router; an Ethernet rate
        double linkDelay @unit(s) = default(0s);                 // each half of the link of host1
        double bottleneckDatarate @unit(bps) = default(100Mbps); // router -- host2; an Ethernet rate
        double bottleneckDelay @unit(s) = default(0s);
        visualizer.typename = default("");
        // The tap is a node without a MAC table, which the MAC forwarding configurator cannot
        // handle; a switch learns the addresses from the frames instead.
        macForwardingTableConfigurator.typename = default("");
    submodules:
        host1: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=300,200");
        }
        tap: PacketTap {
            @display("p=450,200");
        }
        router: <default("Router")> like IEthernetNetworkNode {
            @display("p=600,200");
        }
        host2: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=750,200");
        }
    connections:
        host1.ethg++ <--> EthernetLink { datarate = parent.linkDatarate; delay = parent.linkDelay; } <--> tap.a;
        tap.b <--> EthernetLink { datarate = parent.linkDatarate; delay = parent.linkDelay; } <--> router.ethg++;
        router.ethg++ <--> EthernetLink { datarate = parent.bottleneckDatarate; delay = parent.bottleneckDelay; } <--> host2.ethg++;
}
