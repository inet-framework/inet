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
    "StandardHost",
    "EtherBus",
    "FlatNetworkConfigurator";


module ARPTest
    submodules:
        server: StandardHost;
            display: "p=480,198;i=device/server_l";
        router8: Router;
            display: "p=478,100;i=abstract/router";
        router7: Router;
            display: "p=329,100;i=abstract/router";
        router6: Router;
            display: "p=254,100;i=abstract/router";
        bus2: EtherBus;
            parameters:
                positions = "",
                propagationSpeed = 2e8,
                writeScalars = false;
            display: "p=260,40;b=260,10";
        router5: Router;
            display: "p=187,98;i=abstract/router";
        bus1: EtherBus;
            parameters:
                positions = "",
                propagationSpeed = 2e8,
                writeScalars = false;
            display: "p=280,170;b=260,10";
        router4: Router;
            display: "p=386,240;i=abstract/router";
        router3: Router;
            display: "p=320,240;i=abstract/router";
        router2: Router;
            display: "p=247,240;i=abstract/router";
        router1: Router;
            display: "p=182,283;i=abstract/router";
        client: StandardHost;
            display: "p=54,274;i=device/laptop_l";
        configurator: FlatNetworkConfigurator;
            parameters:
                moduleTypes = "Router StandardHost EtherBus",
                nonIPModuleTypes = "EtherBus",
                networkAddress = "10.0.0.0",
                netmask = "255.255.0.0";
            display: "p=46,111;i=block/cogwheel";
    connections:
        server.ethOut++ --> router8.ethIn++;
        router8.ethOut++ --> server.ethIn++;

        bus2.out++ --> router5.ethIn++;
        router5.ethOut++ --> bus2.in++;

        bus2.out++ --> router6.ethIn++;
        router6.ethOut++ --> bus2.in++;

        bus2.out++ --> router7.ethIn++;
        router7.ethOut++ --> bus2.in++;

        router8.ethOut++ --> router7.ethIn++;
        router7.ethOut++ --> router8.ethIn++;

        bus1.out++ --> router1.ethIn++;
        router1.ethOut++ --> bus1.in++;

        bus1.out++ --> router5.ethIn++;
        router5.ethOut++ --> bus1.in++;

        bus1.out++ --> router2.ethIn++;
        router2.ethOut++ --> bus1.in++;

        bus1.out++ --> router3.ethIn++;
        router3.ethOut++ --> bus1.in++;

        bus1.out++ --> router4.ethIn++;
        router4.ethOut++ --> bus1.in++;

        client.ethOut++ --> router1.ethIn++;
        router1.ethOut++ --> client.ethIn++;
endmodule

network arpTest : ARPTest
endnetwork
