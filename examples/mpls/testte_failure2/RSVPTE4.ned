//
// SPDX-License-Identifier: LGPL-3.0-or-later
//



package inet.examples.mpls.testte_failure2;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.mpls.RsvpMplsRouter;


//
// Example network to demonstrate Rsvp-TE.
//
// See the README file in the model directory for scenario description.
//
network RSVPTE4
{
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        LSR1: RsvpMplsRouter {
            parameters:
                @display("p=400,230");
        }
        LSR2: RsvpMplsRouter {
            parameters:
                @display("p=500,330");
        }
        LSR3: RsvpMplsRouter {
            parameters:
                @display("p=500,130");
        }
        LSR4: RsvpMplsRouter {
            parameters:
                @display("p=600,230");
        }
        LSR5: RsvpMplsRouter {
            parameters:
                @display("p=800,230");
        }
        LSR6: RsvpMplsRouter {
            parameters:
                @display("p=700,360");
        }
        LSR7: RsvpMplsRouter {
            parameters:
                @display("p=700,100");
        }
        host1: StandardHost { // client
            parameters:
                @display("p=250,130;i=device/pc2");
        }
        host2: StandardHost { // client
            parameters:
                @display("p=250,230;i=device/pc2");
        }
        host3: StandardHost { // server
            parameters:
                @display("p=900,130;i=device/server");
        }
        host4: StandardHost { // server
            parameters:
                @display("p=900,330;i=device/server");
        }
        host5: StandardHost { // client
            parameters:
                @display("p=250,330;i=device/pc2");
        }
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=100,200;is=s");
        }
    connections:
        LSR1.pppg++ <--> {  delay = 15ms; datarate = 600kbps; } <--> LSR2.pppg++;
        LSR1.pppg++ <--> {  delay = 5ms; datarate = 600kbps; } <--> LSR3.pppg++;
        host2.pppg++ <--> {  delay = 10ms; datarate = 600kbps; } <--> LSR1.pppg++;
        host1.pppg++ <--> {  delay = 10ms; datarate = 600kbps; } <--> LSR1.pppg++;
        LSR2.pppg++ <--> {  delay = 5ms; datarate = 600kbps; } <--> LSR4.pppg++;
        LSR3.pppg++ <--> {  delay = 5ms; datarate = 600kbps; } <--> LSR4.pppg++;
        LSR4.pppg++ <--> {  delay = 5ms; datarate = 600kbps; } <--> LSR5.pppg++;
        LSR5.pppg++ <--> {  delay = 10ms; datarate = 600kbps; } <--> host3.pppg++;
        LSR5.pppg++ <--> {  delay = 10ms; datarate = 600kbps; } <--> host4.pppg++;
        LSR2.pppg++ <--> {  delay = 10ms; datarate = 600kbps; } <--> LSR6.pppg++;
        LSR5.pppg++ <--> {  delay = 10ms; datarate = 600kbps; } <--> LSR6.pppg++;
        LSR3.pppg++ <--> {  delay = 10ms; datarate = 600kbps; } <--> LSR7.pppg++;
        LSR5.pppg++ <--> {  delay = 10ms; datarate = 600kbps; } <--> LSR7.pppg++;
        host5.pppg++ <--> {  delay = 10ms; datarate = 600kbps; } <--> LSR1.pppg++;
}

