//
// Copyright (C) 2004 Emin Ilker Cetinbas (niw3@yahoo.com)
//
// This library is free software, you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

import
    "Router",
    "StandardHost2",
    "EtherBus",
    "FlatNetworkConfigurator",
    "RTEntryEraser";

module ARPTest
    submodules:
        server: StandardHost2;
            display: "p=480,198;i=server1";
        router8: Router;
            display: "p=478,100;i=srouter";
        router7: Router;
            display: "p=329,100;i=srouter";
        router6: Router;
            display: "p=254,98;i=srouter";
        bus2: EtherBus;
            parameters:
                positions = "",
                propagationSpeed = 2e8,
                writeScalars = false;
            display: "p=277,23";
        router5: Router;
            display: "p=187,98;i=srouter";
        bus1: EtherBus;
            parameters:
                positions = "",
                propagationSpeed = 2e8,
                writeScalars = false;
            display: "p=284,171;b=252,16";
        router4: Router;
            display: "p=386,235;i=srouter";
        router3: Router;
            display: "p=320,237;i=srouter";
        router2: Router;
            display: "p=247,240;i=srouter";
        router1: Router;
            display: "p=182,283;i=srouter";
        client: StandardHost2;
            display: "p=54,274;i=laptop2_l";
        configurator: FlatNetworkConfigurator;
            parameters:
                moduleTypes = "Router StandardHost2 EtherBus",
                nonIPModuleTypes = "EtherBus",
                networkAddress = "10.0.0.0",
                netmask = "255.255.0.0";
            display: "p=46,111;i=cogwheel";
        rtEntryEraser: RTEntryEraser;
            parameters:
                modulesToErase = "client server",
                modulesToManipulate = "router2 router3 router4 router6";
            display: "p=118,350;i=cogwheel";
    connections:
        server.out --> datarate 10e6 --> router8.in++;
        router8.out++ --> datarate 10e6 --> server.in;
        router7.out++ --> datarate 10e6 --> bus2.in++;
        bus2.out++ --> datarate 10e6 --> router7.in++;
        router8.out++ --> datarate 10e6 --> router7.in++;
        router7.out++ --> datarate 10e6 --> router8.in++;
        router5.out++ --> datarate 10e6 --> bus2.in++;
        bus2.out++ --> datarate 10e6 --> router5.in++;
        router5.out++ --> datarate 10e6 --> bus1.in++;
        bus1.out --> datarate 10e6 --> router5.in++;
        bus1.out++ --> datarate 10e6 --> router4.in;
        router4.out --> datarate 10e6 --> bus1.in++;
        bus1.out++ --> datarate 10e6 --> router3.in;
        router3.out --> datarate 10e6 --> bus1.in++;
        bus1.out++ --> datarate 10e6 --> router2.in;
        router2.out --> datarate 10e6 --> bus1.in++;
        bus1.out++ --> datarate 10e6 --> router1.in;
        router1.out --> datarate 10e6 --> bus1.in++;
        client.out --> datarate 10e6 --> router1.in++;
        router1.out++ --> datarate 10e6 --> client.in;
        router6.out++ --> datarate 10e6 --> bus2.in++;
        bus2.out++ --> datarate 10e6 --> router6.in++;
endmodule

network theARPTest : ARPTest
endnetwork