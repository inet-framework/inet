//
// Protocol Test Framework for INET -- Phase 6 MITM demo network.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocoltest;

import inet.node.inet.StandardHost;
import inet.node.ethernet.Eth100M;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;

//
// Two hosts on a wired Ethernet link with a PacketTap spliced in the middle, so the
// tap can drop/delay/mutate frames in flight (fault injection). The ProtocolTester
// observes the resulting trace -- e.g. a TCP retransmission after a dropped segment.
//
network ProtocolTestMitmDemo
{
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,50");
        }
        tester: ProtocolTester {
            @display("p=100,150");
        }
        host1: StandardHost {
            @display("p=300,100");
        }
        tap: PacketTap {
            @display("p=400,100");
        }
        host2: StandardHost {
            @display("p=500,100");
        }
    connections:
        host1.ethg++ <--> Eth100M <--> tap.a;
        tap.b <--> Eth100M <--> host2.ethg++;
}
