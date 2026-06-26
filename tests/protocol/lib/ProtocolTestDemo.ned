//
// Protocol Test Framework for INET -- Phase 0 demo network.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocoltest;

import inet.node.inet.StandardHost;
import inet.node.ethernet.Eth100M;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;

//
// Minimal two-host UDP network used to exercise the Phase-0 observer end to end.
// host1 runs a UdpBasicApp sending to host2 (UdpSink) over an Ethernet link; the
// ProtocolTester observes every packet event across the whole network.
//
network ProtocolTestDemo
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
        host2: StandardHost {
            @display("p=500,100");
        }
    connections:
        host1.ethg++ <--> Eth100M <--> host2.ethg++;
}
