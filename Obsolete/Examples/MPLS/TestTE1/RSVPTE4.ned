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
// Example network to demonstrate CSPF algorithm used in \RSVP-TE.
// The network consists of 5 LSR and 4 hosts. Host 1 and 2 have
// different requests of bandwidth level.
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
                in[2],
                out[2];
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
            display: "p=26,98;i=pc";
        host2: StandardHost2;  // client
            display: "p=26,256;i=pc";
        host3: StandardHost2;  // server
            display: "p=610,80;i=server1";
        host4: StandardHost2;  // server
            display: "p=602,248;i=server1";
        ted: TED;
            parameters:
               moduleTypes = "StandardHost2 RSVP_LSR_Node";
            display: "p=148,376;b=24,16";
    connections nocheck:
        host1.in++ <-- delay 0 error 0 datarate 700 <-- LSR1.out++;
        host1.out++ --> delay 0 error 0 datarate 700 --> LSR1.in++;

        host2.in++ <-- delay 0 error 0 datarate 700 <-- LSR1.out++;
        host2.out++ --> delay 0 error 0 datarate 700 --> LSR1.in++;

        LSR1.out++ --> delay 15 error 0 datarate 600 --> LSR2.in++;
        LSR1.in++ <-- delay 15 error 0 datarate 600 <-- LSR2.out++;

        LSR1.out++ --> delay 5 error 0 datarate 120 --> LSR3.in++;
        LSR1.in++ <-- delay 5 error 0 datarate 120 <-- LSR3.out++;

        LSR2.out++ --> delay 5 error 0 datarate 600 --> LSR4.in++;
        LSR2.in++ <-- delay 5 error 0 datarate 600 <-- LSR4.out++;


        LSR3.out++ --> delay 5 error 0 datarate 200 --> LSR4.in++;
        LSR3.in++ <-- delay 5 error 0 datarate 200 <-- LSR4.out++;

        LSR4.out++ --> delay 5 error 0 datarate 700 --> LSR5.in++;
        LSR4.in++ <-- delay 5 error 0 datarate 700 <-- LSR5.out++;


        LSR5.out++ --> delay 0 error 0 datarate 600 --> host3.in++;
        LSR5.in++ <-- delay 10 error 0 datarate 600 <-- host3.out++;

        LSR5.out++ --> delay 0 error 0 datarate 600 --> host4.in++;
        LSR5.in++ <-- delay 10 error 0 datarate 600 <-- host4.out++;

endmodule

network RSVPTE : RSVPTE4
endnetwork


