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


import
    "StandardHost",
    "RSVP_LSR",
    "FailureManager",
    "ScenarioManager",
    "NAMTrace";

//
// Example network to demonstrate \RSVP-TE.
//
// See the README file in the model directory for scenario description.
//
module RSVPTE4
    submodules:
        nam: NAMTrace;
            display: "p=152,280;i=old/floppy1";

        LSR1: RSVP_LSR;
            parameters:
                peers = "ppp0 ppp1";
            gatesizes:
                in[5],
                out[5];
            display: "p=160,167;i=abstract/router";
        LSR2: RSVP_LSR;
            parameters:
                peers = "ppp0 ppp1 ppp2";
            gatesizes:
                in[3],
                out[3];
            display: "p=254,255;i=abstract/router";
        LSR3: RSVP_LSR;
            parameters:
                peers = "ppp0 ppp1 ppp2";
            gatesizes:
                in[3],
                out[3];
            display: "p=253,78;i=abstract/router";
        LSR4: RSVP_LSR;
            parameters:
                peers = "ppp0 ppp1 ppp2";
            gatesizes:
                in[3],
                out[3];
            display: "p=358,167;i=abstract/router";
        LSR5: RSVP_LSR;
            parameters:
                peers = "ppp0 ppp3 ppp4";
            gatesizes:
                in[5],
                out[5];
            display: "p=460,167;i=abstract/router";
        LSR6: RSVP_LSR;
            parameters:
                peers = "ppp0 ppp1";
            gatesizes:
                in[2],
                out[2];
            display: "p=400,300;i=abstract/router";
        LSR7: RSVP_LSR;
            parameters:
                peers = "ppp0 ppp1";
            gatesizes:
                in[2],
                out[2];
            display: "p=400,50;i=abstract/router";
        host1: StandardHost; // client
            display: "p=71,80;i=device/pc2";
        host2: StandardHost; // client
            display: "p=72,153;i=device/pc2";
        host3: StandardHost; // server
            display: "p=570,88;i=device/server";
        host4: StandardHost; // server
            display: "p=562,256;i=device/server";
        host5: StandardHost; // client
            display: "p=73,233;i=device/pc2";
        scenarioManager: ScenarioManager;
            display: "p=150,50;i=block/control_s";
        failureManager: FailureManager;
            display: "p=500,50;i=block/control_s";
    connections nocheck:
        LSR1.out[0] --> delay 15ms datarate 600000 --> LSR2.in[0];
        LSR1.in[0] <-- delay 15ms datarate 600000 <-- LSR2.out[0];

        LSR1.out[1] --> delay 5ms datarate 600000 --> LSR3.in[0];
        LSR1.in[1] <-- delay 5ms datarate 600000 <-- LSR3.out[0];

        host2.in++ <-- delay 10ms datarate 600000 <-- LSR1.out[2];
        host2.out++ --> delay 10ms datarate 600000 --> LSR1.in[2];

        host1.in++ <-- delay 10ms datarate 600000 <-- LSR1.out[3];
        host1.out++ --> delay 10ms datarate 600000 --> LSR1.in[3];

        LSR2.out[1] --> delay 5ms datarate 600000 --> LSR4.in[0];
        LSR2.in[1] <-- delay 5ms datarate 600000 <-- LSR4.out[0];

        LSR3.out[1] --> delay 5ms datarate 600000 --> LSR4.in[2];
        LSR3.in[1] <-- delay 5ms datarate 600000 <-- LSR4.out[2];

        LSR4.out[1] --> delay 5ms datarate 600000 --> LSR5.in[0];
        LSR4.in[1] <-- delay 5ms datarate 600000 <-- LSR5.out[0];

        LSR5.out[1] --> delay 10ms datarate 600000 --> host3.in++;
        LSR5.in[1] <-- delay 10ms datarate 600000 <-- host3.out++;

        LSR5.out[2] --> delay 10ms datarate 600000 --> host4.in++;
        LSR5.in[2] <-- delay 10ms datarate 600000 <-- host4.out++;

        LSR2.out[2] --> delay 10ms datarate 600000 --> LSR6.in[0];
        LSR2.in[2] <-- delay 10ms datarate 600000 <-- LSR6.out[0];

        LSR5.out[3] --> delay 10ms datarate 600000 --> LSR6.in[1];
        LSR5.in[3] <-- delay 10ms datarate 600000 <-- LSR6.out[1];

        LSR3.out[2] --> delay 10ms datarate 600000 --> LSR7.in[0];
        LSR3.in[2] <-- delay 10ms datarate 600000 <-- LSR7.out[0];

        LSR5.out[4] --> delay 10ms datarate 600000 --> LSR7.in[1];
        LSR5.in[4] <-- delay 10ms datarate 600000 <-- LSR7.out[1];

        host5.in++ <-- delay 10ms datarate 600000 <-- LSR1.out[4];
        host5.out++ --> delay 10ms datarate 600000 --> LSR1.in[4];
endmodule

network RSVPTE : RSVPTE4
endnetwork


