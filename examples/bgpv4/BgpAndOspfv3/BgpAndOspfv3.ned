//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.bgpv4.BgpAndOspfv3;

import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ospfv3.Ospfv3Router;
import inet.node.ipv6.StandardHost6;

//
// BGP / OSPFv3 interoperation over IPv6.
//
// Two autonomous systems, each running OSPFv3 as its interior gateway protocol
// (IGP). The three routers of each AS form an iBGP full mesh; the two border
// routers (rA3, rB1) peer over eBGP. There is NO mutual route redistribution:
// OSPFv3 carries the intra-AS prefixes and resolves the (i)BGP next hops, while
// BGP carries the inter-AS prefixes -- the classic BGP-over-an-IGP arrangement.
//
//   hostA -- rA1 -- rA2 -- rA3 ==eBGP== rB1 -- rB2 -- rB3 -- hostB
//            \_______ AS 65001 _______/        \____ AS 65002 ____/
//               (OSPFv3 + iBGP mesh)           (OSPFv3 + iBGP mesh)
//
// OSPFv3 is load-bearing: rA1 and rA3 are not adjacent, so their iBGP session
// and the next-hop resolution for the routes they exchange rely on the OSPFv3
// route through rA2 (and symmetrically in AS 65002).
//
network BgpAndOspfv3
{
    submodules:
        configurator: Ipv6NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
                config = xmldoc("Ipv6Config.xml");
                // on-link (direct) + host default routes only; intra-AS routes come
                // from OSPFv3 and inter-AS routes from BGP
                addRemoteRoutes = false;
        }
        rA1: Ospfv3Router { @display("p=250,200"); }
        rA2: Ospfv3Router { @display("p=370,200"); }
        rA3: Ospfv3Router { @display("p=490,200"); }
        rB1: Ospfv3Router { @display("p=650,200"); }
        rB2: Ospfv3Router { @display("p=770,200"); }
        rB3: Ospfv3Router { @display("p=890,200"); }
        hostA: StandardHost6 { @display("p=250,320;i=device/pc"); }
        hostB: StandardHost6 { @display("p=890,320;i=device/pc"); }
    connections:
        hostA.ethg++ <--> Eth100M <--> rA1.ethg++;
        rA1.ethg++ <--> Eth100M <--> rA2.ethg++;
        rA2.ethg++ <--> Eth100M <--> rA3.ethg++;
        rA3.ethg++ <--> Eth100M <--> rB1.ethg++;   // inter-AS (eBGP) link
        rB1.ethg++ <--> Eth100M <--> rB2.ethg++;
        rB2.ethg++ <--> Eth100M <--> rB3.ethg++;
        rB3.ethg++ <--> Eth100M <--> hostB.ethg++;
}
