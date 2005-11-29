//
// Copyright (C) 2005 Vojtech Janota
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
    "QuaggaRouter",
    "StandardHost",
    "FailureManager",
    "ScenarioManager";


channel fiberline
    delay 1ms;
    datarate 1000000;
endchannel

channel ethernetline
    delay 10ms;
    datarate 100000;
endchannel

module ZebraTest
    parameters:
        n: numeric const;
    submodules:
        scenarioManager: ScenarioManager;
            display: "p=68,50;i=block/control_s";
        failureManager: FailureManager;
            display: "p=152,50;i=block/control_s";
        nam: NAMTrace;
            display: "p=219,50;i=block/cogwheel_s";
        channelInstaller: ChannelInstaller;
            parameters:
                channelClass = "ThruputMeteringChannel",
                channelAttrs = "format=u #N";
            display: "p=284,50;i=block/cogwheel_s";
        cli: StandardHost[n];
            parameters:
                namid = index;
            display: "p=62,146,c;i=device/laptop";
        srv: StandardHost;
            parameters:
                namid = n;
            display: "p=512,242;i=device/server_l";
        r1: QuaggaRouter;
            parameters:
                namid = n + 1;
            display: "p=173,250;i=abstract/router";
        r2: QuaggaRouter;
            parameters:
                namid = n + 2;
            display: "p=284,130;i=abstract/router";
        r3: QuaggaRouter;
            parameters:
                namid = n + 3;
            display: "p=405,242;i=abstract/router";
        r4: QuaggaRouter;
            parameters:
                namid = n + 4;
            display: "p=294,370;i=abstract/router";
    connections nocheck:
        for i=0..n-1 do
            cli[i].out++ --> ethernetline --> r1.in++;
            cli[i].in++ <-- ethernetline <-- r1.out++;
        endfor;

        r1.out++ --> ethernetline --> r2.in++;
        r1.in++ <-- ethernetline <-- r2.out++;

        r2.out++ --> ethernetline --> r3.in++;
        r2.in++ <-- ethernetline <-- r3.out++;

        r3.out++ --> ethernetline --> srv.in++;
        r3.in++ <-- ethernetline <-- srv.out++;

        r1.out++ --> ethernetline --> r4.in++;
        r1.in++ <-- ethernetline <-- r4.out++;

        r3.out++ --> ethernetline --> r4.in++;
        r3.in++ <-- ethernetline <-- r4.out++;
endmodule

network zebraTest : ZebraTest
endnetwork

