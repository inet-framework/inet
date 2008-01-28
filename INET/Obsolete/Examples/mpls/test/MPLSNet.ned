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




//
// FIXME: add documentation here
//
network MPLSNet
{
    submodules:
        LSR1: LDP_LSR {
            parameters:
                id = "LSR1";
                isIR = true;
                isER = false;
                peerNo = 2;
                @display("p=184,95");
            gates:
                in[4];
                out[4];
        };
        LSR2: LDP_LSR {
            parameters:
                id = "LSR2";
                isIR = false;
                isER = false;
                peerNo = 3;
                @display("p=238,247");
            gates:
                in[3];
                out[3];
        };
        LSR3: LDP_LSR {
            parameters:
                id = "LSR3";
                isIR = false;
                isER = false;
                peerNo = 4;
                @display("p=322,95");
            gates:
                in[4];
                out[4];
        };
        LSR4: LDP_LSR {
            parameters:
                id = "LSR4";
                isIR = false;
                isER = false;
                peerNo = 3;
                @display("p=398,247");
            gates:
                in[3];
                out[3];
        };
        LSR5: LDP_LSR {
            parameters:
                id = "LSR5";
                isIR = false;
                isER = true;
                peerNo = 2;
                @display("p=500,95");
            gates:
                in[4];
                out[4];
        };
        host1: StandardHost {  // client
            parameters:
                @display("p=26,98;i=device/pc3_s");
        };
        host2: StandardHost {  // client
            parameters:
                @display("p=26,256;i=device/pc3_s");
        };
        host3: StandardHost {  // server
            parameters:
                @display("p=570,88;i=device/pc3_s");
        };
        host4: StandardHost {  // server
            parameters:
                @display("p=578,240;i=device/pc3_s");
        };
    connections allowunconnected:
        host1.in[0] <-- LSR1.out[0];
        host1.out[0] --> LSR1.in[0];

        host2.in[0] <-- LSR1.out[1];
        host2.out[0] --> LSR1.in[1];

        LSR1.out[2] --> LSR2.in[0];
        LSR1.in[2] <-- LSR2.out[0];

        LSR1.out[3] --> LSR3.in[0];
        LSR1.in[3] <-- LSR3.out[0];

        LSR2.out[1] --> LSR3.in[1];
        LSR2.in[1] <-- LSR3.out[1];

        LSR2.out[2] --> LSR4.in[0];
        LSR2.in[2] <-- LSR4.out[0];

        LSR3.out[2] --> LSR4.in[1];
        LSR3.in[2] <-- LSR4.out[1];

        LSR3.out[3] --> LSR5.in[0];
        LSR3.in[3] <-- LSR5.out[0];

        LSR4.out[2] --> LSR5.in[1];
        LSR4.in[2] <-- LSR5.out[1];


        LSR5.out[2] --> host3.in[0];
        LSR5.in[2] <-- host3.out[0];

        LSR5.out[3] --> host4.in[0];
        LSR5.in[3] <-- host4.out[0];
}


