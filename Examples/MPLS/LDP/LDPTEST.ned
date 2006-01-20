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
    "LDP_LSR",
    "FailureManager",
    "ScenarioManager",
    "NAMTrace";

module LDPTEST
    submodules:
        nam: NAMTrace;
            display: "p=152,280;i=old/floppy1";

        LSR1: LDP_LSR;
            parameters:
		peers = "ppp0 ppp1 ppp2";
            gatesizes:
                in[4],
                out[4];
            display: "p=150,200;i=abstract/router";
        LSR2: LDP_LSR;
            parameters:
		peers = "ppp0 ppp1 ppp2 ppp3";
            gatesizes:
                in[4],
                out[4];
            display: "p=250,200;i=abstract/router";
        LSR3: LDP_LSR;
            parameters:
		peers = "ppp0 ppp1 ppp2";
            gatesizes:
                in[4],
                out[4];
            display: "p=350,200;i=abstract/router";
        LSR4: LDP_LSR;
            parameters:
		peers = "ppp0 ppp1 ppp2";
            gatesizes:
                in[4],
                out[4];
            display: "p=250,100;i=abstract/router";
        LSR5: LDP_LSR;
            parameters:
		peers = "ppp0 ppp1 ppp2";
            gatesizes:
                in[4],
                out[4];
            display: "p=250,300;i=abstract/router";

        host1: StandardHost; // client
            display: "p=40,200;i=device/pc2";
        host2: StandardHost; // client
            display: "p=440,200;i=device/pc2";
        host3: StandardHost; // client
            display: "p=250,10;i=device/pc2";
        host4: StandardHost; // client
            display: "p=250,390;i=device/pc2";

	scenarioManager: ScenarioManager;
	    display: "p=150,50;i=block/control_s";

	failureManager: FailureManager;
	    display: "p=350,50;i=block/control_s";

    connections nocheck:
        LSR1.out[0] --> delay 15ms datarate 600000 --> LSR2.in[0];
        LSR1.in[0] <-- delay 15ms datarate 600000 <-- LSR2.out[0];

        LSR1.out[1] --> delay 5ms datarate 600000 --> LSR4.in[0];
        LSR1.in[1] <-- delay 5ms datarate 600000 <-- LSR4.out[0];

        LSR1.out[2] --> delay 5ms datarate 600000 --> LSR5.in[0];
        LSR1.in[2] <-- delay 5ms datarate 600000 <-- LSR5.out[0];

        LSR2.out[1] --> delay 5ms datarate 600000 --> LSR3.in[0];
        LSR2.in[1] <-- delay 5ms datarate 600000 <-- LSR3.out[0];

        LSR2.out[2] --> delay 5ms datarate 600000 --> LSR4.in[1];
        LSR2.in[2] <-- delay 5ms datarate 600000 <-- LSR4.out[1];

        LSR2.out[3] --> delay 5ms datarate 600000 --> LSR5.in[1];
        LSR2.in[3] <-- delay 5ms datarate 600000 <-- LSR5.out[1];

        LSR3.out[1] --> delay 10ms datarate 600000 --> LSR4.in[2];
        LSR3.in[1] <-- delay 10ms datarate 600000 <-- LSR4.out[2];

        LSR3.out[2] --> delay 10ms datarate 600000 --> LSR5.in[2];
        LSR3.in[2] <-- delay 10ms datarate 600000 <-- LSR5.out[2];

        host1.in++ <-- delay 10ms datarate 600000 <-- LSR1.out[3];
        host1.out++ --> delay 10ms datarate 600000 --> LSR1.in[3];

        host2.in++ <-- delay 10ms datarate 600000 <-- LSR3.out[3];
        host2.out++ --> delay 10ms datarate 600000 --> LSR3.in[3];

        host3.in++ <-- delay 10ms datarate 600000 <-- LSR4.out[3];
        host3.out++ --> delay 10ms datarate 600000 --> LSR4.in[3];

        host4.in++ <-- delay 10ms datarate 600000 <-- LSR5.out[3];
        host4.out++ --> delay 10ms datarate 600000 --> LSR5.in[3];
endmodule

network LDPTest : LDPTEST
endnetwork


