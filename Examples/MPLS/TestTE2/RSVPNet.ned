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
    "TED";

//
// An example of Hop-by-Hop routing. The network consists of 5 LSRs and
// 2 hosts. Host1 establishes path simply based on \IP routing.
//
// See the README file in the model directory for scenario description.
//
module RSVPNet
    submodules:
        LSR1: RSVP_LSR_Node;
            parameters:
                isIR = true,
                isER = false;
            gatesizes:
                in[4],
                out[4];
            display: "p=104,95;i=srouter";
        LSR2: RSVP_LSR_Node;
            parameters:
                isIR = false,
                isER = false;
            gatesizes:
                in[3],
                out[3];
            display: "p=142,271;i=srouter";
        LSR3: RSVP_LSR_Node;
            parameters:
                isIR = false,
                isER = false;
            gatesizes:
                in[4],
                out[4];
            display: "p=266,95;i=srouter";
        LSR4: RSVP_LSR_Node;
            parameters:
                isIR = false,
                isER = false;
            gatesizes:
                in[3],
                out[3];
            display: "p=422,271;i=srouter";
        LSR5: RSVP_LSR_Node;
            parameters:
                isIR = false,
                isER = true;
            gatesizes:
                in[4],
                out[4];
            display: "p=500,95;i=srouter";
        host1: StandardHost2;  // client
            gatesizes:
                in[1],
                out[1];
            display: "p=26,98;i=pc";
        host3: StandardHost2;  // server
            gatesizes:
                in[1],
                out[1];
            display: "p=570,88;i=server1";
        ted: TED;
            parameters:
               moduleTypes = "StandardHost2 RSVP_LSR_Node";
            display: "p=276,320;b=24,16";
    connections nocheck:
        host1.in[0] <-- delay 0ms datarate 9000 <-- LSR1.out[0];
        host1.out[0] --> delay 0ms datarate 9000 --> LSR1.in[0];

        LSR1.out[2] --> delay 1ms datarate 2000 --> LSR2.in[0];
        LSR1.in[2] <-- delay 1ms datarate 1000 <-- LSR2.out[0];

        LSR1.out[3] --> delay 5ms datarate 1000 --> LSR3.in[0];
        LSR1.in[3] <-- delay 5ms datarate 1000 <-- LSR3.out[0];

        LSR2.out[1] --> delay 5ms datarate 1000 --> LSR3.in[1];
        LSR2.in[1] <-- delay 5ms datarate 2000 <-- LSR3.out[1];

        LSR2.out[2] --> delay 15ms datarate 4000 --> LSR4.in[0];
        LSR2.in[2] <-- delay 15ms datarate 1000 <-- LSR4.out[0];

        LSR3.out[2] --> delay 15ms datarate 2000 --> LSR4.in[1];
        LSR3.in[2] <-- delay 15ms datarate 1000 <-- LSR4.out[1];

        LSR3.out[3] --> delay 5ms datarate 2000 --> LSR5.in[0];
        LSR3.in[3] <-- delay 5ms datarate 2000 <-- LSR5.out[0];

        LSR4.out[2] --> delay 2ms datarate 1000 --> LSR5.in[1];
        LSR4.in[2] <-- delay 2ms datarate 2000 <-- LSR5.out[1];


        LSR5.out[2] --> delay 0ms datarate 9000 --> host3.in[0];
        LSR5.in[2] <-- delay 10ms datarate 9000 <-- host3.out[0];

endmodule

network RSVPTest : RSVPNet
endnetwork


