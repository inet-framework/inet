//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.mpls.te;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.mpls.RsvpMplsRouter;

//
// An 8-LSR RSVP-TE traffic engineering network used to demonstrate explicit-route
// LSPs with bandwidth reservation, setup/holding priority preemption, and
// Hello-detected failure recovery. See the showcase documentation for details.
//
network MplsTeShowcase
{
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=60,60;is=s");
        }
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=60,120;is=s");
        }
        host1: StandardHost { // client, sends to host3 over the high-priority tunnel
            parameters:
                @display("p=60,260;i=device/pc2");
        }
        host2: StandardHost { // client, sends to host4 over the low-priority tunnel
            parameters:
                @display("p=60,460;i=device/pc2");
        }
        LSR1: RsvpMplsRouter { // ingress
            parameters:
                @display("p=220,360");
        }
        LSR2: RsvpMplsRouter {
            parameters:
                @display("p=380,180");
        }
        LSR3: RsvpMplsRouter {
            parameters:
                @display("p=380,540");
        }
        LSR4: RsvpMplsRouter { // shut down/restarted in the FailureReroute config
            parameters:
                @display("p=540,180");
        }
        LSR5: RsvpMplsRouter { // merge point before egress
            parameters:
                @display("p=700,360");
        }
        LSR6: RsvpMplsRouter { // spare bypass capacity around LSR4 (unused by the
                                // hand-written EROs below: this model has no CSPF to
                                // route around a failure automatically)
            parameters:
                @display("p=540,60");
        }
        LSR7: RsvpMplsRouter {
            parameters:
                @display("p=540,540");
        }
        LSR8: RsvpMplsRouter { // egress
            parameters:
                @display("p=860,360");
        }
        host3: StandardHost { // server, high-priority tunnel's endpoint
            parameters:
                @display("p=1020,260;i=device/server");
        }
        host4: StandardHost { // server, low-priority tunnel's endpoint
            parameters:
                @display("p=1020,460;i=device/server");
        }
    connections:
        host1.pppg++ <--> {  delay = 10ms; datarate = 600kbps; } <--> LSR1.pppg++;
        host2.pppg++ <--> {  delay = 10ms; datarate = 600kbps; } <--> LSR1.pppg++;
        LSR1.pppg++ <--> {  delay = 5ms; datarate = 600kbps; } <--> LSR2.pppg++;
        LSR1.pppg++ <--> {  delay = 5ms; datarate = 600kbps; } <--> LSR3.pppg++;
        LSR2.pppg++ <--> {  delay = 5ms; datarate = 600kbps; } <--> LSR4.pppg++;
        LSR2.pppg++ <--> {  delay = 5ms; datarate = 600kbps; } <--> LSR6.pppg++;
        LSR3.pppg++ <--> {  delay = 5ms; datarate = 600kbps; } <--> LSR4.pppg++;
        LSR3.pppg++ <--> {  delay = 5ms; datarate = 600kbps; } <--> LSR7.pppg++;
        LSR4.pppg++ <--> {  delay = 5ms; datarate = 600kbps; } <--> LSR5.pppg++;
        LSR6.pppg++ <--> {  delay = 5ms; datarate = 600kbps; } <--> LSR5.pppg++;
        LSR7.pppg++ <--> {  delay = 5ms; datarate = 600kbps; } <--> LSR5.pppg++;
        // LSR5-LSR8 carries both tunnels' merged traffic (400+400 kbit/s) in the
        // baseline configuration, so it needs more capacity than the other,
        // single-tunnel backbone links (which stay at 600 kbit/s so that the
        // PriorityPreemption config's extra 400 kbit/s path can genuinely
        // exceed their capacity and trigger preemption).
        LSR5.pppg++ <--> {  delay = 5ms; datarate = 1Mbps; } <--> LSR8.pppg++;
        LSR8.pppg++ <--> {  delay = 10ms; datarate = 600kbps; } <--> host3.pppg++;
        LSR8.pppg++ <--> {  delay = 10ms; datarate = 600kbps; } <--> host4.pppg++;
}
