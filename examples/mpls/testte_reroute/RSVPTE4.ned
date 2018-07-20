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



package inet.examples.mpls.testte_reroute;

import inet.common.scenario.ScenarioManager;
import inet.node.inet.StandardHost;
import inet.node.mpls.RsvpMplsRouter;


//
// Example network to demonstrate Rsvp-TE.
//
// See the README file in the model directory for scenario description.
//
network RSVPTE4
{
    parameters:
        **.ipv4.configurator.networkConfiguratorModule = "";

    submodules:
        LSR1: RsvpMplsRouter {
            parameters:
                peers = "ppp0 ppp1";
                @display("p=400,230");
            gates:
                pppg[5];
        }
        LSR2: RsvpMplsRouter {
            parameters:
                peers = "ppp0 ppp1 ppp2";
                @display("p=500,330");
            gates:
                pppg[3];
        }
        LSR3: RsvpMplsRouter {
            parameters:
                peers = "ppp0 ppp1 ppp2";
                @display("p=500,130");
            gates:
                pppg[3];
        }
        LSR4: RsvpMplsRouter {
            parameters:
                peers = "ppp0 ppp1 ppp2";
                @display("p=600,230");
            gates:
                pppg[3];
        }
        LSR5: RsvpMplsRouter {
            parameters:
                peers = "ppp0 ppp3 ppp4";
                @display("p=800,230");
            gates:
                pppg[5];
        }
        LSR6: RsvpMplsRouter {
            parameters:
                peers = "ppp0 ppp1";
                @display("p=700,360");
            gates:
                pppg[2];
        }
        LSR7: RsvpMplsRouter {
            parameters:
                peers = "ppp0 ppp1";
                @display("p=700,100");
            gates:
                pppg[2];
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
                @display("p=100,100;is=s");
        }
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

