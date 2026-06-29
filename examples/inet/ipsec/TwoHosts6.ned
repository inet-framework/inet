//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.inet.ipsec;

import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.ipv6.StandardHost6;
import ned.DatarateChannel;

//
// IPv6 counterpart of ~TwoHosts: two IPv6 hosts on a point-to-point link, used to
// demonstrate IPsec (AH/ESP, transport mode) over IPv6. The Ipv6NetworkConfigurator
// assigns global addresses so the hosts are reachable by name.
//
network TwoHosts6
{
    submodules:
        client1: StandardHost6 {
            @display("p=50,60;i=device/pc3");
        }
        server: StandardHost6 {
            @display("p=300,60;i=device/pc2");
        }
        configurator: Ipv6NetworkConfigurator {
            @display("p=53,134");
        }
    connections:
        client1.pppg++ <--> { datarate = 10Mbps; delay = 0.1us; } <--> server.pppg++;
}
