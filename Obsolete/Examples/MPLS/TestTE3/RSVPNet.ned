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
// An example of Explicit Routing in RSVP-TE. The network consists of
// 5 LSRs and 2 hosts. Host2 establishes its path based on the input
// of ROUTE specified in the traffic.xml file.
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
        host2: StandardHost2;  // client
            parameters:
            display: "p=26,256;i=comp_s";
        host4: StandardHost2;  // server
            parameters:
            display: "p=578,240;i=comp_s";
        ted: TED;
            display: "p=276,320;b=24,16";
    connections nocheck:

        host2.in[0] <-- delay 0 error 0 datarate 9000 <-- LSR1.out[1];
        host2.out[0] --> delay 0 error 0 datarate 9000 --> LSR1.in[1];

        LSR1.out[2] --> delay 1 error 0 datarate 2000 --> LSR2.in[0];
        LSR1.in[2] <-- delay 1 error 0 datarate 1000 <-- LSR2.out[0];

        LSR1.out[3] --> delay 5 error 0 datarate 1000 --> LSR3.in[0];
        LSR1.in[3] <-- delay 5 error 0 datarate 1000 <-- LSR3.out[0];

        LSR2.out[1] --> delay 5 error 0 datarate 1000 --> LSR3.in[1];
        LSR2.in[1] <-- delay 5 error 0 datarate 2000 <-- LSR3.out[1];

        LSR2.out[2] --> delay 15 error 0 datarate 4000 --> LSR4.in[0];
        LSR2.in[2] <-- delay 15 error 0 datarate 1000 <-- LSR4.out[0];

        LSR3.out[2] --> delay 15 error 0 datarate 2000 --> LSR4.in[1];
        LSR3.in[2] <-- delay 15 error 0 datarate 1000 <-- LSR4.out[1];

        LSR3.out[3] --> delay 5 error 0 datarate 2000 --> LSR5.in[0];
        LSR3.in[3] <-- delay 5 error 0 datarate 2000 <-- LSR5.out[0];

        LSR4.out[2] --> delay 2 error 0 datarate 1000 --> LSR5.in[1];
        LSR4.in[2] <-- delay 2 error 0 datarate 2000 <-- LSR5.out[1];


        LSR5.out[3] --> delay 0 error 0 datarate 9000 --> host4.in[0];
        LSR5.in[3] <-- delay 10 error 0 datarate 9000 <-- host4.out[0];

endmodule

network RSVPTest : RSVPNet
endnetwork


