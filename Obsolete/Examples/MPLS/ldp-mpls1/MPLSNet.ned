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
    "LDP_LSR",
    "StandardHost";


//
// FIXME: add documentation here
//
module MPLSNet
    submodules:
        LSR1: LDP_LSR;
            parameters:
                isIR = true,
                isER = false,
                peerNo = 2;
            gatesizes:
                in[4],
                out[4];
            display: "p=184,95;i=abstract/router";
        LSR2: LDP_LSR;
            parameters:
                isIR = false,
                isER = false,
                peerNo = 3;
            gatesizes:
                in[3],
                out[3];
            display: "p=238,247;i=abstract/router";
        LSR3: LDP_LSR;
            parameters:
                isIR = false,
                isER = false,
                peerNo = 4;
            gatesizes:
                in[4],
                out[4];
            display: "p=322,95;i=abstract/router";
        LSR4: LDP_LSR;
            parameters:
                isIR = false,
                isER = false,
                peerNo = 3;
            gatesizes:
                in[3],
                out[3];
            display: "p=398,247;i=abstract/router";
        LSR5: LDP_LSR;
            parameters:
                isIR = false,
                isER = true,
                peerNo = 2;
            gatesizes:
                in[4],
                out[4];
            display: "p=500,95;i=abstract/router";
        host1: StandardHost;  // client
            gatesizes:
                in[1],
                out[1];
            display: "p=26,98;i=device/pc2";
        host2: StandardHost;  // client
            gatesizes:
                in[1],
                out[1];
            display: "p=26,256;i=device/pc2";
        host3: StandardHost;  // server
            gatesizes:
                in[1],
                out[1];
            display: "p=570,88;i=device/pc2";
        host4: StandardHost;  // server
            gatesizes:
                in[1],
                out[1];
            display: "p=578,240;i=device/pc2";
    connections nocheck:
        host1.in[0] <-- datarate 64000 <-- LSR1.out[0];
        host1.out[0] --> datarate 64000 --> LSR1.in[0];

        host2.in[0] <-- datarate 64000 <-- LSR1.out[1];
        host2.out[0] --> datarate 64000 --> LSR1.in[1];

        LSR1.out[2] --> datarate 64000 --> LSR2.in[0];
        LSR1.in[2] <-- datarate 64000 <-- LSR2.out[0];

        LSR1.out[3] --> datarate 64000 --> LSR3.in[0];
        LSR1.in[3] <-- datarate 64000 <-- LSR3.out[0];

        LSR2.out[1] --> datarate 64000 --> LSR3.in[1];
        LSR2.in[1] <-- datarate 64000 <-- LSR3.out[1];

        LSR2.out[2] --> datarate 64000 --> LSR4.in[0];
        LSR2.in[2] <-- datarate 64000 <-- LSR4.out[0];

        LSR3.out[2] --> datarate 64000 --> LSR4.in[1];
        LSR3.in[2] <-- datarate 64000 <-- LSR4.out[1];

        LSR3.out[3] --> datarate 64000 --> LSR5.in[0];
        LSR3.in[3] <-- datarate 64000 <-- LSR5.out[0];

        LSR4.out[2] --> datarate 64000 --> LSR5.in[1];
        LSR4.in[2] <-- datarate 64000 <-- LSR5.out[1];


        LSR5.out[2] --> datarate 64000 --> host3.in[0];
        LSR5.in[2] <-- datarate 64000 <-- host3.out[0];

        LSR5.out[3] --> datarate 64000 --> host4.in[0];
        LSR5.in[3] <-- datarate 64000 <-- host4.out[0];

endmodule

network mplsTest : MPLSNet
endnetwork
