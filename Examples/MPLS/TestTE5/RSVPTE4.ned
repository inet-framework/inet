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
    "NewBWRequestTester",
    "TED";

//
// Another example of traffic rerouting.
//
// See the README file in the model directory for scenario description.
//
module RSVPTE4
    submodules:
        LSR1: RSVP_LSR;
            parameters:
                isIR = true,
                isER = false;
            gatesizes:
                in[4],
                out[4];
            display: "p=155,133;i=abstract/router";
        LSR2: RSVP_LSR;
            parameters:
                isIR = false,
                isER = false;
            gatesizes:
                in[3],
                out[3];
            display: "p=246,214;i=abstract/router";
        LSR3: RSVP_LSR;
            parameters:
                isIR = false,
                isER = false;
            gatesizes:
                in[2],
                out[2];
            display: "p=242,63;i=abstract/router";
        LSR4: RSVP_LSR;
            parameters:
                isIR = false,
                isER = false;
            gatesizes:
                in[3],
                out[3];
            display: "p=334,126;i=abstract/router";
        LSR5: RSVP_LSR;
            parameters:
                isIR = false,
                isER = true;
            gatesizes:
                in[3],
                out[3];
            display: "p=436,126;i=abstract/router";
        host1: StandardHost; // client
            gatesizes:
                in[1],
                out[1];
            display: "p=58,136;i=device/pc2";
        host3: StandardHost; // server
            gatesizes:
                in[1],
                out[1];
            display: "p=523,125;i=device/server";
        ted: TED;
            parameters:
                moduleTypes = "StandardHost RSVP_LSR";
            display: "p=397,230;i=block/table";
        tester: NewBWRequestTester;
            parameters:
                startTime = 100;
            display: "p=477,229;i=block/cogwheel";
    connections nocheck:
        host1.in[0] <-- delay 0ms datarate 700 <-- LSR1.out[0];
        host1.out[0] --> delay 0ms datarate 700 --> LSR1.in[0];

        LSR1.out[2] --> delay 5ms datarate 600 --> LSR2.in[0];
        LSR1.in[2] <-- delay 5ms datarate 600 <-- LSR2.out[0];

        LSR1.out[3] --> delay 25ms datarate 120 --> LSR3.in[0];
        LSR1.in[3] <-- delay 25ms datarate 120 <-- LSR3.out[0];

        LSR2.out[2] --> delay 5ms datarate 200 --> LSR4.in[0];
        LSR2.in[2] <-- delay 5ms datarate 200 <-- LSR4.out[0];


        LSR3.out[1] --> delay 5ms datarate 200 --> LSR4.in[1];
        LSR3.in[1] <-- delay 5ms datarate 200 <-- LSR4.out[1];

        LSR4.out[2] --> delay 5ms datarate 150 --> LSR5.in[0];
        LSR4.in[2] <-- delay 5ms datarate 150 <-- LSR5.out[0];


        LSR5.out[1] --> delay 0ms datarate 600 --> host3.in[0];
        LSR5.in[1] <-- delay 10ms datarate 600 <-- host3.out[0];

endmodule

network TrafficReroute : RSVPTE4
endnetwork


