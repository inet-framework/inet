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
    "StandardHost2",
    "RSVP_LSR_Node",
    "Tester",
    "TED";

//
// LSP preemption example.
//
// FIXME there's a very similar NED file in the same directry --
// one of them has to be removed?
//
// See the README file in the model directory for scenario description.
//
module RSVPTE4
    submodules:
        LSR1: RSVP_LSR_Node;
            parameters:
                isIR = true,
                isER = false;
            gatesizes:
                in[4],
                out[4];
            display: "p=112,175;i=srouter";
        LSR2: RSVP_LSR_Node;
            parameters:
                isIR = false,
                isER = false;
            gatesizes:
                in[3],
                out[3];
            display: "p=206,263;i=srouter";
        LSR3: RSVP_LSR_Node;
            parameters:
                isIR = false,
                isER = false;
            gatesizes:
                in[2],
                out[2];
            display: "p=186,95;i=srouter";
        LSR4: RSVP_LSR_Node;
            parameters:
                isIR = false,
                isER = false;
            gatesizes:
                in[3],
                out[3];
            display: "p=286,159;i=srouter";
        LSR5: RSVP_LSR_Node;
            parameters:
                isIR = false,
                isER = true;
            gatesizes:
                in[3],
                out[3];
            display: "p=388,159;i=srouter";
        host1: StandardHost2;  // client
            gatesizes:
                in[1],
                out[1];
            display: "p=26,98;i=pc";
        host2: StandardHost2;  // client
            gatesizes:
                in[1],
                out[1];
            display: "p=26,256;i=pc";
        host3: StandardHost2;  // server
            gatesizes:
                in[1],
                out[1];
            display: "p=500,96;i=server1";
        host4: StandardHost2;  // server
            gatesizes:
                in[1],
                out[1];
            display: "p=500,248;i=server1";
        ted: TED;
            parameters:
               moduleTypes = "StandardHost2 RSVP_LSR_Node";
            display: "p=320,300;i=proc2";
    connections nocheck:
        host1.in[0] <-- delay 0ms datarate 700 <-- LSR1.out[0];
        host1.out[0] --> delay 0ms datarate 700 --> LSR1.in[0];
        host2.in[0] <-- delay 0ms datarate 700 <-- LSR1.out[1];
        host2.out[0] --> delay 0ms datarate 700 --> LSR1.in[1];

        LSR1.out[2] --> delay 25ms datarate 600 --> LSR2.in[0];
        LSR1.in[2] <-- delay 25ms datarate 600 <-- LSR2.out[0];

        LSR1.out[3] --> delay 5ms datarate 120 --> LSR3.in[0];
        LSR1.in[3] <-- delay 5ms datarate 120 <-- LSR3.out[0];

        LSR2.out[2] --> delay 5ms datarate 200 --> LSR4.in[0];
        LSR2.in[2] <-- delay 5ms datarate 200 <-- LSR4.out[0];


        LSR3.out[1] --> delay 5ms datarate 200 --> LSR4.in[1];
        LSR3.in[1] <-- delay 5ms datarate 200 <-- LSR4.out[1];

        LSR4.out[2] --> delay 5ms datarate 150 --> LSR5.in[0];
        LSR4.in[2] <-- delay 5ms datarate 150 <-- LSR5.out[0];


        LSR5.out[1] --> delay 0ms datarate 600 --> host3.in[0];
        LSR5.in[1] <-- delay 10ms datarate 600 <-- host3.out[0];

        LSR5.out[2] --> delay 0ms datarate 600 --> host4.in[0];
        LSR5.in[2] <-- delay 10ms datarate 600 <-- host4.out[0];

endmodule

network Preemption : RSVPTE4
endnetwork


