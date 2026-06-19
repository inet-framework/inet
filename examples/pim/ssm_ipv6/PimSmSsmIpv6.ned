//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.pim.ssm_ipv6;

import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ipv6.MulticastRouter6;
import inet.node.ipv6.StandardHost6;

//
// Pure-IPv6 PIM-SM source-specific multicast (SSM, RFC 4607) topology. There is
// NO rendezvous point: SSM groups (the FF3x::/32 range) are served by (S,G)
// source trees built directly toward the source.
//
//     Source1 --\
//                R1 ----- R2 ----- Receiver
//     Source2 --/
//
// Both sources send to the same SSM group ff3e::8. The Receiver (behind R2) uses
// MLDv2 to subscribe in INCLUDE mode to Source1 only. R2 therefore sends a
// PIM-SM Join(Source1, ff3e::8) toward the source (via R1, bypassing any RP),
// and the Receiver gets Source1's stream -- but never Source2's, since no
// (Source2, ff3e::8) tree is ever built.
//
network PimSmSsmIpv6
{
    submodules:
        Source1: StandardHost6 {
            @display("p=200,150;i=device/server");
        }
        Source2: StandardHost6 {
            @display("p=200,300;i=device/server");
        }
        R1: MulticastRouter6 { // the sources' designated router
            @display("p=350,225");
        }
        R2: MulticastRouter6 { // the receiver's designated router
            @display("p=500,225");
        }
        Receiver: StandardHost6 {
            @display("p=650,225;i=device/pc");
        }
        configurator: Ipv6FlatNetworkConfigurator {
            @display("p=100,100;is=s");
        }
    connections allowunconnected:
        Source1.ethg++ <--> Eth100M <--> R1.ethg++;
        Source2.ethg++ <--> Eth100M <--> R1.ethg++;
        R1.ethg++ <--> Eth100M <--> R2.ethg++;
        R2.ethg++ <--> Eth100M <--> Receiver.ethg++;
}
