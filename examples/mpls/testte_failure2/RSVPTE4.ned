//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//



package inet.examples.mpls.testte_failure2;

import inet.base.UnimplementedModule;
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.extras.FailureManager;
import inet.nodes.inet.StandardHost;
import inet.nodes.mpls.RSVP_LSR;
import inet.world.scenario.ScenarioManager;


//
// Example network to demonstrate RSVP-TE.
//
// See the README file in the model directory for scenario description.
//
network RSVPTE4
{
    submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=115,50");
        }
        LSR1: RSVP_LSR {
            parameters:
                @display("p=160,167");
        }
        LSR2: RSVP_LSR {
            parameters:
                @display("p=254,255");
        }
        LSR3: RSVP_LSR {
            parameters:
                @display("p=253,78");
        }
        LSR4: RSVP_LSR {
            parameters:
                @display("p=358,167");
        }
        LSR5: RSVP_LSR {
            parameters:
                @display("p=460,167");
        }
        LSR6: RSVP_LSR {
            parameters:
                @display("p=400,300");
        }
        LSR7: RSVP_LSR {
            parameters:
                @display("p=400,50");
        }
        host1: StandardHost { // client
            parameters:
                @display("p=71,80;i=device/pc2");
        }
        host2: StandardHost { // client
            parameters:
                @display("p=72,153;i=device/pc2");
        }
        host3: StandardHost { // server
            parameters:
                @display("p=570,88;i=device/server");
        }
        host4: StandardHost { // server
            parameters:
                @display("p=562,256;i=device/server");
        }
        host5: StandardHost { // client
            parameters:
                @display("p=73,233;i=device/pc2");
        }
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=195,50");
        }
        failureManager: FailureManager {
            parameters:
                @display("p=500,50");
        }
        configurator: UnimplementedModule;
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

