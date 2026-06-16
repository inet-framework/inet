//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.ipv6.mld;

import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
import inet.node.ethernet.EthernetHub;
import inet.node.ipv6.Router6;
import inet.node.ipv6.StandardHost6;
import ned.DatarateChannel;

//
// MLDv1 demonstration network: two listener hosts and a router querier share an
// Ethernet hub. The hosts join a multicast group (ff0e::1) via UdpSink and the
// router querier sends periodic General Queries. One host leaves the group,
// triggering an MLD Done -> Multicast-Address-Specific Query sequence.
//
// Run command (from this directory):
//   inet -u Cmdenv -f omnetpp.ini -c MldDemo -r 0
//
network MldDemo
{
    types:
        channel ethline extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        configurator: Ipv6FlatNetworkConfigurator {
            @display("p=100,80;is=s");
        }
        router: Router6 {
            @display("p=400,100");
        }
        host1: StandardHost6 {
            @display("p=200,300");
        }
        host2: StandardHost6 {
            @display("p=600,300");
        }
        sender: StandardHost6 {
            @display("p=700,100");
        }
        hub: EthernetHub {
            @display("p=400,200;is=s");
        }
    connections:
        router.ethg++ <--> ethline <--> hub.ethg++;
        host1.ethg++ <--> ethline <--> hub.ethg++;
        host2.ethg++ <--> ethline <--> hub.ethg++;
        sender.ethg++ <--> ethline <--> hub.ethg++;
}
