//
// Copyright (C) 2006 Andras Varga
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
    "EtherSwitch",
    "FlatNetworkConfigurator";

channel fiberline
    delay 1us;
    datarate 512*1000000;
endchannel


module ARPTest
    submodules:
        client: StandardHost;
            display: "p=71,64;i=device/laptop_l";
        host1: StandardHost;
            display: "p=65,131;i=device/pc";
        host2: StandardHost;
            display: "p=60,191;i=device/pc";
        switch: EtherSwitch;
            display: "p=202,156;i=device/switch";
        net: Router;
            display: "p=394,166;i=misc/cloud_l";
        router: Router;
            display: "p=311,74;i=abstract/router";
        server: StandardHost;
            display: "p=512,58;i=device/server_l";
        configurator: FlatNetworkConfigurator;
            parameters:
                moduleTypes = "Router StandardHost EtherSwitch",
                nonIPModuleTypes = "EtherSwitch",
                networkAddress = "10.0.0.0",
                netmask = "255.255.0.0";
            display: "p=495,160;i=block/cogwheel_s";
    connections:
        client.ethOut++ --> switch.in++;
        switch.out++ --> client.ethIn++;

        switch.out++ --> host1.ethIn++;
        host1.ethOut++ --> switch.in++;

        switch.out++ --> host2.ethIn++;
        host2.ethOut++ --> switch.in++;

        router.ethOut++ --> switch.in++;
        switch.out++ --> router.ethIn++;

        router.out++ --> fiberline --> net.in++;
        net.out++ --> fiberline --> router.in++;

        server.out++ --> fiberline --> net.in++;
        net.out++ --> fiberline --> server.in++;
endmodule

network arpTest : ARPTest
endnetwork

