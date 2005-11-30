//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


import
    "Router",
    "StandardHost",
    "FlatNetworkConfigurator",
    "ScenarioManager",
    "ChannelInstaller";

module REDTest
    submodules:
        channelInstaller: ChannelInstaller;
            parameters:
                channelClass = "ThruputMeteringChannel",
                channelAttrs = "format=b (u) #N";
            display: "p=359,40;i=block/cogwheel_s";
        configurator: FlatNetworkConfigurator;
            parameters:
                moduleTypes = "Router StandardHost",
                nonIPModuleTypes = "",
                networkAddress = "145.236.0.0",
                netmask = "255.255.0.0";
            display: "p=179,40;i=block/cogwheel_s";
        scenarioManager: ScenarioManager;
            display: "p=262,40;i=block/control_s";
        r1: Router;
            display: "p=147,143;i=abstract/router";
        r2: Router;
            display: "p=317,143;i=abstract/router";
        s1: StandardHost;
            display: "p=68,64;i=device/laptop";
        s2: StandardHost;
            display: "p=65,193;i=device/laptop";
        s3: StandardHost;
            display: "p=421,83;i=device/laptop";
        s4: StandardHost;
            display: "p=420,195;i=device/laptop";
    connections nocheck:
        r1.out++ --> datarate 1.5e6 delay 20e-3 --> r2.in++;
        r1.in++ <-- datarate 1.5e6 delay 20e-3 <-- r2.out++;

        s1.out++ --> datarate 10e6 delay 2e-3 --> r1.in++;
        s1.in++ <-- datarate 10e6 delay 2e-3 <-- r1.out++;

        s2.out++ --> datarate 10e6 delay 3e-3 --> r1.in++;
        s2.in++ <-- datarate 10e6 delay 3e-3 <-- r1.out++;

        s3.out++ --> datarate 10e6 delay 4e-3 --> r2.in++;
        s3.in++ <-- datarate 10e6 delay 4e-3 <-- r2.out++;

        s4.out++ --> datarate 10e6 delay 5e-3 --> r2.in++;
        s4.in++ <-- datarate 10e6 delay 5e-3 <-- r2.out++;

endmodule

network redTest : REDTest
endnetwork

