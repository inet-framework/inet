//
// IPv6 conformance suite -- one shared LAN: an advertising Router6 and two
// StandardHost6 hosts on a single Ethernet link (via an EthernetSwitch, so all three
// share one IPv6 subnet). This one link naturally exercises the whole boot sequence
// the suite observes:
//   * each node forms a link-local address and runs Duplicate Address Detection
//     (RFC 4862) -- an NS from the unspecified source to the solicited-node multicast;
//   * the router sends Router Advertisements and the hosts send Router Solicitations
//     (RFC 4861 router discovery);
//   * the hosts autoconfigure a global address from the advertised prefix (SLAAC) and
//     run DAD on it too;
//   * a host->host UDP flow triggers address resolution (NS/NA) on the link.
//
// Because host1 and host2 are on the same link, the suite can also force a duplicate
// (identical MAC -> identical link-local) to observe DAD failure, and can watch
// neighbour address resolution between the two hosts.
//
// The per-test knobs (SLAAC vs. DAD-disabled, same-MAC collision, optimistic DAD,
// bootup ordering) are set per .test in its %inifile; ini/_base.ini holds the shared
// SLAAC configuration. The ProtocolTester observes the ND message + DAD-signal trace.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
package inet.protocoltest.ipv6;

import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.ethernet.EthernetSwitch;
import inet.node.ipv6.Router6;
import inet.node.ipv6.StandardHost6;
import inet.protocoltest.ProtocolTester;
import ned.DatarateChannel;

network Ipv6LanNetwork
{
    types:
        channel ethline extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        configurator: Ipv6NetworkConfigurator {
            @display("p=100,50;is=s");
        }
        tester: ProtocolTester {
            @display("p=100,150");
        }
        switch: EthernetSwitch {
            @display("p=350,150");
        }
        router: Router6 {
            @display("p=350,50");
        }
        host1: StandardHost6 {
            @display("p=250,260;i=device/pc");
        }
        host2: StandardHost6 {
            @display("p=450,260;i=device/pc");
        }
    connections:
        router.ethg++ <--> ethline <--> switch.ethg++;
        host1.ethg++ <--> ethline <--> switch.ethg++;
        host2.ethg++ <--> ethline <--> switch.ethg++;
}
