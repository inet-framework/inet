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



package inet.examples.mpls.testte_routing;

import inet.base.UnimplementedModule;
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
        LSR1: RSVP_LSR {
            parameters:
                peers = "ppp0 ppp1";
                @display("p=160,167");
            gates:
                pppg[5];
        }
        LSR2: RSVP_LSR {
            parameters:
                peers = "ppp0 ppp1 ppp2";
                @display("p=254,255");
            gates:
                pppg[3];
        }
        LSR3: RSVP_LSR {
            parameters:
                peers = "ppp0 ppp1 ppp2";
                @display("p=253,78");
            gates:
                pppg[3];
        }
        LSR4: RSVP_LSR {
            parameters:
                peers = "ppp0 ppp1 ppp2";
                @display("p=358,167");
            gates:
                pppg[3];
        }
        LSR5: RSVP_LSR {
            parameters:
                peers = "ppp0 ppp3 ppp4";
                @display("p=460,167");
            gates:
                pppg[5];
        }
        LSR6: RSVP_LSR {
            parameters:
                peers = "ppp0 ppp1";
                @display("p=400,300");
            gates:
                pppg[2];
        }
        LSR7: RSVP_LSR {
            parameters:
                peers = "ppp0 ppp1";
                @display("p=400,50");
            gates:
                pppg[2];
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
                @display("p=150,50");
        }
        failureManager: FailureManager {
            parameters:
                @display("p=500,50");
        }
        configurator: UnimplementedModule;
    connections:
        LSR1.pppg[0] <--> {  delay = 15ms; datarate = 600kbps; } <--> LSR2.pppg[0];
        LSR1.pppg[1] <--> {  delay = 5ms; datarate = 600kbps; } <--> LSR3.pppg[0];
        host2.pppg++ <--> {  delay = 10ms; datarate = 600kbps; } <--> LSR1.pppg[2];
        host1.pppg++ <--> {  delay = 10ms; datarate = 600kbps; } <--> LSR1.pppg[3];
        LSR2.pppg[1] <--> {  delay = 5ms; datarate = 600kbps; } <--> LSR4.pppg[0];
        LSR3.pppg[1] <--> {  delay = 5ms; datarate = 600kbps; } <--> LSR4.pppg[2];
        LSR4.pppg[1] <--> {  delay = 5ms; datarate = 600kbps; } <--> LSR5.pppg[0];
        LSR5.pppg[1] <--> {  delay = 10ms; datarate = 600kbps; } <--> host3.pppg++;
        LSR5.pppg[2] <--> {  delay = 10ms; datarate = 600kbps; } <--> host4.pppg++;
        LSR2.pppg[2] <--> {  delay = 10ms; datarate = 600kbps; } <--> LSR6.pppg[0];
        LSR5.pppg[3] <--> {  delay = 10ms; datarate = 600kbps; } <--> LSR6.pppg[1];
        LSR3.pppg[2] <--> {  delay = 10ms; datarate = 600kbps; } <--> LSR7.pppg[0];
        LSR5.pppg[4] <--> {  delay = 10ms; datarate = 600kbps; } <--> LSR7.pppg[1];
        host5.pppg++ <--> {  delay = 10ms; datarate = 600kbps; } <--> LSR1.pppg[4];
}

