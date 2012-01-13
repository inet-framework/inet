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



package inet.examples.mpls.ldp;

import inet.networklayer.extras.FailureManager;
import inet.nodes.inet.StandardHost;
import inet.nodes.mpls.LDP_LSR;
import inet.world.NAMTrace;
import inet.world.ScenarioManager;
import ned.DatarateChannel;


network LDPTEST
{
    types:
        channel C5 extends DatarateChannel
        {
            datarate = 600kbps;
            delay = 5us;
        }
        channel C10 extends DatarateChannel
        {
            datarate = 600kbps;
            delay = 10us;
        }
        channel C15 extends DatarateChannel
        {
            datarate = 600kbps;
            delay = 15us;
        }
    submodules:
        nam: NAMTrace {
            parameters:
                @display("p=152,280");
        }
        LSR1: LDP_LSR {
            parameters:
                peers = "ppp0 ppp1 ppp2";
                @display("p=150,200");
            gates:
                pppg[4];
        }
        LSR2: LDP_LSR {
            parameters:
                peers = "ppp0 ppp1 ppp2 ppp3";
                @display("p=250,200");
            gates:
                pppg[4];
        }
        LSR3: LDP_LSR {
            parameters:
                peers = "ppp0 ppp1 ppp2";
                @display("p=350,200");
            gates:
                pppg[4];
        }
        LSR4: LDP_LSR {
            parameters:
                peers = "ppp0 ppp1 ppp2";
                @display("p=250,100");
            gates:
                pppg[4];
        }
        LSR5: LDP_LSR {
            parameters:
                peers = "ppp0 ppp1 ppp2";
                @display("p=250,300");
            gates:
                pppg[4];
        }
        host1: StandardHost { // client
            parameters:
                @display("p=40,200;i=device/pc2");
        }
        host2: StandardHost { // client
            parameters:
                @display("p=440,200;i=device/pc2");
        }
        host3: StandardHost { // client
            parameters:
                @display("p=250,10;i=device/pc2");
        }
        host4: StandardHost { // client
            parameters:
                @display("p=250,390;i=device/pc2");
        }
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=150,50");
        }
        failureManager: FailureManager {
            parameters:
                @display("p=350,50");
        }
    connections:
        LSR1.pppg[0] <--> C15 <--> LSR2.pppg[0];
        LSR1.pppg[1] <--> C5 <--> LSR4.pppg[0];
        LSR1.pppg[2] <--> C5 <--> LSR5.pppg[0];
        LSR2.pppg[1] <--> C5 <--> LSR3.pppg[0];
        LSR2.pppg[2] <--> C5 <--> LSR4.pppg[1];
        LSR2.pppg[3] <--> C5 <--> LSR5.pppg[1];
        LSR3.pppg[1] <--> C10 <--> LSR4.pppg[2];
        LSR3.pppg[2] <--> C10 <--> LSR5.pppg[2];
        host1.pppg++ <--> C10 <--> LSR1.pppg[3];
        host2.pppg++ <--> C10 <--> LSR3.pppg[3];
        host3.pppg++ <--> C10 <--> LSR4.pppg[3];
        host4.pppg++ <--> C10 <--> LSR5.pppg[3];
}

