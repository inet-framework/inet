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
    "TCPClientTest",
    "TCPServerTest",
    "RSVP_LSR_Node",
    "Tester",
    "TED";

//
// Example network to demonstrate CSPF algorithm used in RSVP-TE.
// The network consists of 5 LSR and 4 hosts. Host 1 and 2 have
// different requests of bandwidth level.
//
// See the README file in the model directory for scenario description.
//
module RSVPTE4
    parameters:

    submodules:
        LSR1: RSVP_LSR_Node;
            parameters:
                isIR = true,
                isER = false,
                numOfPorts = 4;
            gatesizes:
                in[4],
                out[4];
            display: "p=112,175;i=srouter";
        LSR2: RSVP_LSR_Node;
            parameters:
                isIR = false,
                isER = false,
                numOfPorts = 2;
            gatesizes:
                in[2],
                out[2];
            display: "p=206,263;i=srouter";
        LSR3: RSVP_LSR_Node;
            parameters:
                isIR = false,
                isER = false,
                numOfPorts = 2;
            gatesizes:
                in[2],
                out[2];
            display: "p=186,95;i=srouter";
        LSR4: RSVP_LSR_Node;
            parameters:
                isIR = false,
                isER = false,
                numOfPorts = 3;
            gatesizes:
                in[3],
                out[3];
            display: "p=286,159;i=srouter";
        LSR5: RSVP_LSR_Node;
            parameters:
                isIR = false,
                isER = true,
                numOfPorts = 3;
            gatesizes:
                in[3],
                out[3];
            display: "p=388,159;i=srouter";
        host1: TCPClientTest;
            parameters:
                start_time = 0,
                numOfPorts = 1;
            display: "p=26,98;i=comp_s";
        host2: TCPClientTest;
            parameters:
                start_time = 0,
                numOfPorts = 1;
            display: "p=26,256;i=comp_s";
        host3: TCPServerTest;
            parameters:
                numOfPorts = 1;
            display: "p=610,80;i=comp_s";
        host4: TCPServerTest;
            parameters:
                numOfPorts = 1;
            display: "p=602,248;i=comp_s";
        ted: TED;
            display: "p=148,376;b=24,16";
    connections nocheck:
        host1.in[0] <-- delay 0 error 0 datarate 700 <-- LSR1.out[0];
        host1.out[0] --> delay 0 error 0 datarate 700 --> LSR1.in[0];

        host2.in[0] <-- delay 0 error 0 datarate 700 <-- LSR1.out[1];
        host2.out[0] --> delay 0 error 0 datarate 700 --> LSR1.in[1];

        LSR1.out[2] --> delay 15 error 0 datarate 600 --> LSR2.in[0];
        LSR1.in[2] <-- delay 15 error 0 datarate 600 <-- LSR2.out[0];

        LSR1.out[3] --> delay 5 error 0 datarate 120 --> LSR3.in[0];
        LSR1.in[3] <-- delay 5 error 0 datarate 120 <-- LSR3.out[0];

        LSR2.out[1] --> delay 5 error 0 datarate 600 --> LSR4.in[0];
        LSR2.in[1] <-- delay 5 error 0 datarate 600 <-- LSR4.out[0];


        LSR3.out[1] --> delay 5 error 0 datarate 200 --> LSR4.in[1];
        LSR3.in[1] <-- delay 5 error 0 datarate 200 <-- LSR4.out[1];

        LSR4.out[2] --> delay 5 error 0 datarate 700 --> LSR5.in[0];
        LSR4.in[2] <-- delay 5 error 0 datarate 700 <-- LSR5.out[0];


        LSR5.out[2] --> delay 0 error 0 datarate 600 --> host3.in[0];
        LSR5.in[2] <-- delay 10 error 0 datarate 600 <-- host3.out[0];

        LSR5.out[1] --> delay 0 error 0 datarate 600 --> host4.in[0];
        LSR5.in[1] <-- delay 10 error 0 datarate 600 <-- host4.out[0];

endmodule

network RSVPTE : RSVPTE4
endnetwork


