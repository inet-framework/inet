//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.mpls.pcep;

import inet.common.scenario.ScenarioManager;
import inet.node.mpls.RsvpMplsRouter;
import inet.node.inet.StandardHost;

//
// A 6-LSR RSVP-TE core, delegated to a remote stateful PCE (Workstream F4):
// host1 -- LSR1 -> {LSR2 -> LSR3 (600 kbps branch, metric 1+1), LSR4 -> LSR5
// (10 Mbps branch, metric 5+5)} -> LSR6 -- host2, plus PceRouter, a seventh
// node hanging off LSR6 on its own leaf link, running the ~Pce -- deliberately
// not co-located with LSR6 itself, mirroring tests/module/pcep_fallback.test's
// own placement rationale (so the PCE's own reachability is never entangled
// with the LSPs' data-plane path).
//
// LSR1 delegates its ingress LSP to the PCE (~RsvpTe's "computationMode" =
// "pce", "delegate" = true); see this directory's README for the full
// PCE-driven reoptimization scenario this network is built to demonstrate.
//
network PcepMplsTe
{
    parameters:
        **.ipv4.configurator.networkConfiguratorModule = "";
    submodules:
        host1: StandardHost {
            @display("p=50,300");
            gates: pppg[1];
        }
        host2: StandardHost {
            @display("p=900,300");
            gates: pppg[1];
        }
        LSR1: RsvpMplsRouter {
            @display("p=200,300");
            gates: pppg[3];
        }
        LSR2: RsvpMplsRouter {
            @display("p=400,150");
            gates: pppg[2];
        }
        LSR3: RsvpMplsRouter {
            @display("p=550,150");
            gates: pppg[2];
        }
        LSR4: RsvpMplsRouter {
            @display("p=400,450");
            gates: pppg[2];
        }
        LSR5: RsvpMplsRouter {
            @display("p=550,450");
            gates: pppg[2];
        }
        LSR6: RsvpMplsRouter {
            @display("p=750,300");
            gates: pppg[4];
        }
        PceRouter: RsvpMplsRouter {
            @display("p=750,150");
            gates: pppg[1];
        }
        scenarioManager: ScenarioManager {
            @display("p=100,100;is=s");
        }
    connections:
        host1.pppg[0] <--> { delay = 1ms; datarate = 10Mbps; } <--> LSR1.pppg[0];
        LSR1.pppg[1] <--> { delay = 1ms; datarate = 600kbps; } <--> LSR2.pppg[0];
        LSR2.pppg[1] <--> { delay = 1ms; datarate = 600kbps; } <--> LSR3.pppg[0];
        LSR3.pppg[1] <--> { delay = 1ms; datarate = 600kbps; } <--> LSR6.pppg[0];
        LSR1.pppg[2] <--> { delay = 1ms; datarate = 10Mbps; } <--> LSR4.pppg[0];
        LSR4.pppg[1] <--> { delay = 1ms; datarate = 10Mbps; } <--> LSR5.pppg[0];
        LSR5.pppg[1] <--> { delay = 1ms; datarate = 10Mbps; } <--> LSR6.pppg[1];
        LSR6.pppg[2] <--> { delay = 1ms; datarate = 10Mbps; } <--> PceRouter.pppg[0];
        LSR6.pppg[3] <--> { delay = 1ms; datarate = 10Mbps; } <--> host2.pppg[0];
}
