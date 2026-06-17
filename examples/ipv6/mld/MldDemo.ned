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
// MLD demonstration network: listener hosts, a UDP sender, and a router querier
// share an Ethernet hub. Used by two configs in omnetpp.ini: MldDemo (MLDv1,
// any-source: hosts join ff0e::1, the querier sends General Queries, a leave
// triggers an MLD Done -> Multicast-Address-Specific Query) and MldV2Ssm (MLDv2
// source-specific multicast: a receiver subscribes to one of two sources of an
// ff3e:: SSM group in INCLUDE mode).
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
