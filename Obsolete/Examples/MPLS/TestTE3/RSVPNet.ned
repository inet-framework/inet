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
    "TED";


//
// An example of Explicit Routing in \RSVP-TE. The network consists of
// 5 LSRs and 2 hosts. Host2 establishes its path based on the input
// of ROUTE specified in the traffic.xml file.
//
// See the README file in the model directory for scenario description.
//
module RSVPNet
    submodules:
        LSR1: RSVP_LSR;
            parameters:
                isIR = true,
                isER = false;
            gatesizes:
                in[4],
                out[4];
            display: "p=165,68;i=abstract/router";
        LSR2: RSVP_LSR;
            parameters:
                isIR = false,
                isER = false;
            gatesizes:
                in[3],
                out[3];
            display: "p=203,244;i=abstract/router";
        LSR3: RSVP_LSR;
            parameters:
                isIR = false,
                isER = false;
            gatesizes:
                in[4],
                out[4];
            display: "p=344,68;i=abstract/router";
        LSR4: RSVP_LSR;
            parameters:
                isIR = false,
                isER = false;
            gatesizes:
                in[3],
                out[3];
            display: "p=483,244;i=abstract/router";
        LSR5: RSVP_LSR;
            parameters:
                isIR = false,
                isER = true;
            gatesizes:
                in[4],
                out[4];
            display: "p=561,68;i=abstract/router";
        host2: StandardHost; // client
            gatesizes:
                in[1],
                out[1];
            display: "p=65,159;i=device/pc2";
        host4: StandardHost; // server
            gatesizes:
                in[1],
                out[1];
            display: "p=643,155;i=device/server";
        ted: TED;
            parameters:
                moduleTypes = "StandardHost RSVP_LSR";
            display: "p=97,262;i=block/table";
    connections nocheck:

        host2.in[0] <-- delay 0ms datarate 9000 <-- LSR1.out[1];
        host2.out[0] --> delay 0ms datarate 9000 --> LSR1.in[1];

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


        LSR5.out[3] --> delay 0ms datarate 9000 --> host4.in[0];
        LSR5.in[3] <-- delay 10ms datarate 9000 <-- host4.out[0];

endmodule

network RSVPTest : RSVPNet
endnetwork


