//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocoltest.coap;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EthernetLink;
import inet.node.inet.StandardHost;
import inet.protocoltest.ProtocolTester;

//
// Shared IPv4 pair network for CoAP protocol conformance tests.
//
network CoapPairNetwork
{
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,50");
        }
        tester: ProtocolTester {
            @display("p=100,150");
        }
        hostA: StandardHost {
            @display("p=300,100");
        }
        hostB: StandardHost {
            @display("p=500,100");
        }
    connections:
        hostA.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 0.1ms; } <--> hostB.ethg++;
}
