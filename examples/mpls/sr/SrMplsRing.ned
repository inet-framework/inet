//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.mpls.sr;

import inet.common.scenario.ScenarioManager;
import inet.node.inet.StandardHost;
import inet.node.mpls.SrMplsRouter;
import ned.DatarateChannel;

//
// A six-router "ring + chord" SR-MPLS (RFC 8660) network, demonstrating all
// three ways an ~SrPolicy can steer traffic:
//
// - a plain node-SID policy (just names the destination's node SID and lets
//   the network's own shortest-path computation pick the route -- here, the
//   R1-R3 chord, a 2-hop shortcut across the ring);
// - an explicit-path policy (names an intermediate waypoint's node SID to
//   force a longer, non-IGP-preferred route -- SR traffic engineering
//   without any signaling protocol);
// - an adjacency-SID pin (names one of R1's own two parallel links to R2 by
//   its LOCAL interface, forcing a specific physical link regardless of its
//   IGP cost).
//
// See the README for the full walk-through and expected forwarding behavior.
//
network SrMplsRing
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
            @display("p=100,150");
        }
        R1: SrMplsRouter {
            parameters:
                routerId = "1.1.1.1";
                peers = "ppp1 ppp2 ppp3 ppp4";
                sidTable = xmldoc("sidtable.xml");
                policies = xmldoc("policies.xml");
                @display("p=300,150");
            gates:
                pppg[5];
        }
        R2: SrMplsRouter {
            parameters:
                routerId = "2.2.2.2";
                peers = "ppp0 ppp1 ppp2";
                sidTable = xmldoc("sidtable.xml");
                @display("p=500,80");
            gates:
                pppg[3];
        }
        R3: SrMplsRouter {
            parameters:
                routerId = "3.3.3.3";
                peers = "ppp0 ppp1 ppp2";
                sidTable = xmldoc("sidtable.xml");
                @display("p=700,150");
            gates:
                pppg[3];
        }
        R4: SrMplsRouter {
            parameters:
                routerId = "4.4.4.4";
                peers = "ppp0 ppp1";
                sidTable = xmldoc("sidtable.xml");
                @display("p=760,380");
            gates:
                pppg[5];
        }
        R5: SrMplsRouter {
            parameters:
                routerId = "5.5.5.5";
                peers = "ppp0 ppp1";
                sidTable = xmldoc("sidtable.xml");
                @display("p=550,500");
            gates:
                pppg[2];
        }
        R6: SrMplsRouter {
            parameters:
                routerId = "6.6.6.6";
                peers = "ppp0 ppp1";
                sidTable = xmldoc("sidtable.xml");
                @display("p=330,440");
            gates:
                pppg[2];
        }
        hostPlain: StandardHost {
            @display("p=960,300");
        }
        hostExplicit: StandardHost {
            @display("p=960,380");
        }
        hostAdj: StandardHost {
            @display("p=960,460");
        }
        scenarioManager: ScenarioManager { // idle by default (empty script); see the [TiLfa] config
            @display("p=100,460");
        }
    connections:
        hostA.pppg++ <--> Link <--> R1.pppg[0];
        R1.pppg[1] <--> Link <--> R2.pppg[0]; // primary R1<->R2 link, metric 1 (IGP-preferred)
        R1.pppg[2] <--> Link <--> R6.pppg[1]; // ring edge
        R1.pppg[3] <--> Link <--> R3.pppg[2]; // chord: R1<->R3 shortcut, metric 1
        R1.pppg[4] <--> Link <--> R2.pppg[2]; // secondary R1<->R2 link, metric 10 (adjacency-SID pin target)
        R2.pppg[1] <--> Link <--> R3.pppg[0]; // ring edge
        R3.pppg[1] <--> Link <--> R4.pppg[0]; // ring edge
        R4.pppg[1] <--> Link <--> R5.pppg[0]; // ring edge
        R5.pppg[1] <--> Link <--> R6.pppg[0]; // ring edge
        R4.pppg[2] <--> Link <--> hostPlain.pppg++;
        R4.pppg[3] <--> Link <--> hostExplicit.pppg++;
        R4.pppg[4] <--> Link <--> hostAdj.pppg++;
}
