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
    "NetworkConfigurator",
    "StandardHost",
    "RSVP_LSR",
    "FailureManager",
    "ScenarioManager";

//
// Example network to demonstrate \RSVP-TE.
//
// See the README file in the model directory for scenario description.
//
module RSVPTE4
    submodules:
        configurator: NetworkConfigurator;
            parameters:
                moduleTypes = "RSVP_LSR StandardHost";
            display: "p=115,50;i=block/cogwheel_s";
        LSR1: RSVP_LSR;
            display: "p=160,167;i=abstract/router";
        LSR2: RSVP_LSR;
            display: "p=254,255;i=abstract/router";
        LSR3: RSVP_LSR;
            display: "p=253,78;i=abstract/router";
        LSR4: RSVP_LSR;
            display: "p=358,167;i=abstract/router";
        LSR5: RSVP_LSR;
            display: "p=460,167;i=abstract/router";
        LSR6: RSVP_LSR;
            display: "p=400,300;i=abstract/router";
        LSR7: RSVP_LSR;
            display: "p=400,50;i=abstract/router";
        host1: StandardHost; // client
            display: "p=71,80;i=device/pc2";
        host2: StandardHost; // client
            display: "p=72,153;i=device/pc2";
        host3: StandardHost; // server
            display: "p=570,88;i=device/server";
        host4: StandardHost; // server
            display: "p=562,256;i=device/server";
        host5: StandardHost; // client
            display: "p=73,233;i=device/pc2";
        scenarioManager: ScenarioManager;
            display: "p=195,50;i=block/control_s";
        failureManager: FailureManager;
            display: "p=500,50;i=block/control_s";
        //nam: NAMTraceWriter;
        //    display: "p=152,280;i=old/floppy1";
    connections nocheck:
        LSR1.out++ --> delay 15ms datarate 600000 --> LSR2.in++;
        LSR1.in++ <-- delay 15ms datarate 600000 <-- LSR2.out++;

        LSR1.out++ --> delay 5ms datarate 600000 --> LSR3.in++;
        LSR1.in++ <-- delay 5ms datarate 600000 <-- LSR3.out++;

        host2.in++ <-- delay 10ms datarate 600000 <-- LSR1.out++;
        host2.out++ --> delay 10ms datarate 600000 --> LSR1.in++;

        host1.in++ <-- delay 10ms datarate 600000 <-- LSR1.out++;
        host1.out++ --> delay 10ms datarate 600000 --> LSR1.in++;

        LSR2.out++ --> delay 5ms datarate 600000 --> LSR4.in++;
        LSR2.in++ <-- delay 5ms datarate 600000 <-- LSR4.out++;

        LSR3.out++ --> delay 5ms datarate 600000 --> LSR4.in++;
        LSR3.in++ <-- delay 5ms datarate 600000 <-- LSR4.out++;

        LSR4.out++ --> delay 5ms datarate 600000 --> LSR5.in++;
        LSR4.in++ <-- delay 5ms datarate 600000 <-- LSR5.out++;

        LSR5.out++ --> delay 10ms datarate 600000 --> host3.in++;
        LSR5.in++ <-- delay 10ms datarate 600000 <-- host3.out++;

        LSR5.out++ --> delay 10ms datarate 600000 --> host4.in++;
        LSR5.in++ <-- delay 10ms datarate 600000 <-- host4.out++;

        LSR2.out++ --> delay 10ms datarate 600000 --> LSR6.in++;
        LSR2.in++ <-- delay 10ms datarate 600000 <-- LSR6.out++;

        LSR5.out++ --> delay 10ms datarate 600000 --> LSR6.in++;
        LSR5.in++ <-- delay 10ms datarate 600000 <-- LSR6.out++;

        LSR3.out++ --> delay 10ms datarate 600000 --> LSR7.in++;
        LSR3.in++ <-- delay 10ms datarate 600000 <-- LSR7.out++;

        LSR5.out++ --> delay 10ms datarate 600000 --> LSR7.in++;
        LSR5.in++ <-- delay 10ms datarate 600000 <-- LSR7.out++;

        host5.in++ <-- delay 10ms datarate 600000 <-- LSR1.out++;
        host5.out++ --> delay 10ms datarate 600000 --> LSR1.in++;
endmodule

network RSVPTE : RSVPTE4
endnetwork


