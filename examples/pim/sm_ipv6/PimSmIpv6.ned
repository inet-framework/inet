//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.pim.sm_ipv6;

import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ipv6.MulticastRouter6;
import inet.node.ipv6.StandardHost6;

//
// Pure-IPv6 PIM-SM (Sparse Mode) topology demonstrating the rendezvous-point
// machinery over IPv6: Register/Register-Stop, the shared (*,G) tree, and the
// switchover to the (S,G) source tree.
//
// The three routers form a triangle so that the shortest path from the receiver
// side to the source does NOT go through the RP:
//
//     Source --- DR_S ===== RP
//                  \\       //
//                   \\     //
//                    == DR_R --- Receiver
//
// DR_S is the source's designated router, RP is the (statically configured)
// rendezvous point, DR_R is the receiver's designated router. When the Source
// starts sending, DR_S encapsulates the traffic in PIM Register messages to the
// RP; the RP forwards it down the shared tree to the Receiver and joins the
// source tree. The last-hop router DR_R then switches to the shortest-path tree,
// joining (S,G) toward the Source over the direct DR_S--DR_R link (bypassing the
// RP).
//
network PimSmIpv6
{
    submodules:
        DR_S: MulticastRouter6 { // source's designated router
            @display("p=400,200");
        }
        RP: MulticastRouter6 { // rendezvous point
            @display("p=550,120");
        }
        DR_R: MulticastRouter6 { // receiver's designated router
            @display("p=550,300");
        }
        Source: StandardHost6 {
            @display("p=250,200;i=device/server");
        }
        Receiver: StandardHost6 {
            @display("p=700,300;i=device/pc");
        }
        configurator: Ipv6FlatNetworkConfigurator {
            @display("p=100,100;is=s");
        }
    connections allowunconnected:
        Source.ethg++ <--> Eth100M <--> DR_S.ethg++;
        DR_S.ethg++ <--> Eth100M <--> RP.ethg++;
        DR_S.ethg++ <--> Eth100M <--> DR_R.ethg++; // direct link enabling a distinct SPT
        RP.ethg++ <--> Eth100M <--> DR_R.ethg++;
        DR_R.ethg++ <--> Eth100M <--> Receiver.ethg++;
}
