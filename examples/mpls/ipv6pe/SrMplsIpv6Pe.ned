//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.mpls.ipv6pe;

import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.mpls.SrMplsRouter;
import ned.DatarateChannel;

//
// A "6PE-lite" demo: two IPv6-addressed edge islands (hostA behind R1, and
// hostB/hostC behind R4) reached across a small, purely IPv4-addressed,
// IPv4-signaled SR-MPLS (RFC 8660) core -- edge routers R1/R4 carry an extra
// IPv6-addressed interface toward their local island; the core routers R2/R3
// and R1/R4's OWN core-facing interfaces stay plain IPv4, exactly as in a
// real 6PE deployment's P/PE split. See the README for the full walk-through,
// what this demo does and does NOT model relative to real 6PE (RFC 4798),
// and for the Workstream F3 Phase 4 fix (inner IPv6 explicit-null label) that
// makes this work at all over SR's family-agnostic node-SID PHP pops.
//
// Topology: R1-R2-R3-R4 chain plus an R1-R3 chord (all links metric 1),
// mirroring examples/mpls/sr/'s "ring + chord" shape at a much smaller
// scale -- just enough for the chord to give hostA->hostB's UNCONSTRAINED
// policy a genuine 2-hop shortcut (PUSH at R1, PHP POP at R3, R2 never
// touched), while hostA->hostC's EXPLICIT-PATH policy names R2 as a waypoint
// and is forced the long way around (PUSH at R1, genuine transit SWAP at R2,
// PHP POP at R3) -- the same qualitative divergence examples/mpls/sr's first
// two flows demonstrate, here carrying IPv6 traffic instead of IPv4.
//
network SrMplsIpv6Pe
{
    parameters:
        **.ipv4.configurator.networkConfiguratorModule = "";
    types:
        channel Link extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 100us;
        }
    submodules:
        hostA: StandardHost {
            hasIpv6 = true;
            @display("p=100,200");
        }
        R1: SrMplsRouter {
            hasIpv6 = true; // edge: also carries an IPv6 address toward hostA
            routerId = "1.1.1.1";
            peers = "ppp1 ppp2"; // core-facing interfaces only (excludes hostA-facing ppp0)
            sidTable = xmldoc("sidtable.xml");
            policies = xmldoc("policies.xml");
            @display("p=300,200");
            gates:
                pppg[3];
        }
        R2: SrMplsRouter {
            // pure-IPv4 transit: hasIpv6 stays false (the default) -- no IPv6 awareness
            // whatsoever, yet it still correctly transits the explicit-path flow's IPv6
            // payload (see README and Workstream F3 Phase 4's fix).
            routerId = "2.2.2.2";
            peers = "ppp0 ppp1";
            sidTable = xmldoc("sidtable.xml");
            @display("p=500,100");
            gates:
                pppg[2];
        }
        R3: SrMplsRouter {
            // pure-IPv4 transit too, despite doing PHP for BOTH flows (see README).
            routerId = "3.3.3.3";
            peers = "ppp0 ppp1 ppp2";
            sidTable = xmldoc("sidtable.xml");
            @display("p=500,300");
            gates:
                pppg[3];
        }
        R4: SrMplsRouter {
            hasIpv6 = true; // edge: also carries IPv6 addresses toward hostB and hostC
            routerId = "4.4.4.4";
            peers = "ppp0"; // core-facing interface only (excludes hostB/hostC-facing ppp1/ppp2)
            sidTable = xmldoc("sidtable.xml");
            @display("p=700,200");
            gates:
                pppg[3];
        }
        hostB: StandardHost {
            hasIpv6 = true;
            @display("p=900,120");
        }
        hostC: StandardHost {
            hasIpv6 = true;
            @display("p=900,280");
        }
        configurator: Ipv6NetworkConfigurator {
            @display("p=400,420");
        }
    connections:
        hostA.pppg++ <--> Link <--> R1.pppg[0];
        R1.pppg[1] <--> Link <--> R2.pppg[0]; // chain
        R2.pppg[1] <--> Link <--> R3.pppg[1]; // chain
        R3.pppg[2] <--> Link <--> R4.pppg[0]; // chain
        R1.pppg[2] <--> Link <--> R3.pppg[0]; // chord: R1<->R3 shortcut
        R4.pppg[1] <--> Link <--> hostB.pppg++;
        R4.pppg[2] <--> Link <--> hostC.pppg++;
}
