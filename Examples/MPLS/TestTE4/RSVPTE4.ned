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
    "NewRouteTester",
    "TED";


//
// An example of traffic rerouting.
//
// After the LSP has been established from host1 to host3, there is a change
// in the network resource. The delay of the link between LSR2 and
// LSR1 drops . This triggers CSPF to recalculate a more optimal path.
// A new LSP path is setup and traffic is rerouted from the old LSP
// path to the new LSP path.
//
// This is an example of "make-before-break" technique used in \RSVP-TE for
// traffic rerouting.
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
            display: "p=173,132;i=abstract/router";
        LSR2: RSVP_LSR;
            parameters:
                isIR = false,
                isER = false;
            gatesizes:
                in[3],
                out[3];
            display: "p=255,213;i=abstract/router";
        LSR3: RSVP_LSR;
            parameters:
                isIR = false,
                isER = false;
            gatesizes:
                in[2],
                out[2];
            display: "p=255,61;i=abstract/router";
        LSR4: RSVP_LSR;
            parameters:
                isIR = false,
                isER = false;
            gatesizes:
                in[3],
                out[3];
            display: "p=346,132;i=abstract/router";
        LSR5: RSVP_LSR;
            parameters:
                isIR = false,
                isER = true;
            gatesizes:
                in[3],
                out[3];
            display: "p=450,132;i=abstract/router";
        host1: StandardHost; // client
            gatesizes:
                in[1],
                out[1];
            display: "p=51,133;i=device/pc2";
        host3: StandardHost; // server
            gatesizes:
                in[1],
                out[1];
            display: "p=552,131;i=device/server";
        ted: TED;
            parameters:
                moduleTypes = "StandardHost RSVP_LSR";
            display: "p=426,237;i=block/table";
        tester: NewRouteTester;
            parameters:
                startTime = 100;
            display: "p=488,237;i=block/cogwheel";
    connections nocheck:
        host1.in[0] <-- delay 0ms datarate 700 <-- LSR1.out[0];
        host1.out[0] --> delay 0ms datarate 700 --> LSR1.in[0];

        LSR1.out[2] --> delay 5ms datarate 600 --> LSR2.in[0];
        LSR1.in[2] <-- delay 5ms datarate 600 <-- LSR2.out[0];

        LSR1.out[3] --> delay 25ms datarate 120 --> LSR3.in[0];
        LSR1.in[3] <-- delay 25ms datarate 120 <-- LSR3.out[0];

        LSR2.out[2] --> delay 5ms datarate 60 --> LSR4.in[0];
        LSR2.in[2] <-- delay 5ms datarate 60 <-- LSR4.out[0];

        LSR3.out[1] --> delay 5ms datarate 200 --> LSR4.in[1];
        LSR3.in[1] <-- delay 5ms datarate 200 <-- LSR4.out[1];

        LSR4.out[2] --> delay 5ms datarate 150 --> LSR5.in[0];
        LSR4.in[2] <-- delay 5ms datarate 150 <-- LSR5.out[0];


        LSR5.out[1] --> delay 0ms datarate 600 --> host3.in[0];
        LSR5.in[1] <-- delay 10ms datarate 600 <-- host3.out[0];

endmodule

network TrafficReroute : RSVPTE4
endnetwork


