//
// IPv6 conformance suite -- routed (multi-hop) network for the behaviours that need a
// router in the path: forwarding + Hop Limit handling, ICMPv6 error generation
// (Time Exceeded, Packet Too Big, Destination Unreachable), Path MTU Discovery,
// source fragmentation, Neighbour Unreachability Detection of a next-hop router, and
// ICMPv6 Redirect.
//
// Topology (modelled on INET's own tests/module IPv6_redirect): host1, routerA and
// routerB share one link via a switch; host2 sits on a second link behind routerB.
// Because both routers are on host1's link, the flat configurator produces routes where
// a router forwards a packet back out the arrival interface -- which is exactly what
// makes routerA send host1 an ICMPv6 Redirect towards routerB (RFC 4861 8). host1's
// traffic to host2 (off-link) also exercises Hop Limit decrement, ICMPv6 errors, and
// NUD of the next-hop router.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
package inet.protocoltest.ipv6;

import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
import inet.node.ethernet.EthernetSwitch;
import inet.node.ipv6.Router6;
import inet.node.ipv6.StandardHost6;
import inet.protocoltest.ProtocolTester;
import ned.DatarateChannel;

network Ipv6RoutedNetwork
{
    types:
        channel ethline extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        configurator: Ipv6FlatNetworkConfigurator {
            @display("p=100,50;is=s");
        }
        tester: ProtocolTester {
            @display("p=100,150");
        }
        switch: EthernetSwitch {
            @display("p=300,200");
        }
        routerA: Router6 {
            @display("p=300,80");
        }
        routerB: Router6 {
            @display("p=450,120");
        }
        host1: StandardHost6 {
            @display("p=180,280;i=device/pc");
        }
        host2: StandardHost6 {
            @display("p=600,200;i=device/pc");
        }
    connections:
        host1.ethg++ <--> ethline <--> switch.ethg++;
        routerA.ethg++ <--> ethline <--> switch.ethg++;
        routerB.ethg++ <--> ethline <--> switch.ethg++;
        routerB.ethg++ <--> ethline <--> host2.ethg++;
}
