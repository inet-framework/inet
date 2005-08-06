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
    "FlatNetworkConfigurator";


channel fiberline
    delay 1us;
    datarate 512*1000000;
endchannel

channel ethernetline
    delay 0.1us;
    datarate 10*1000000;
endchannel

module DLLTest
    parameters:
        n: numeric const;
    submodules:
        configurator: FlatNetworkConfigurator;
            parameters:
                moduleTypes = "Router StandardHost",
                nonIPModuleTypes = "",
                networkAddress = "145.236.0.0",
                netmask = "255.255.0.0";
            display: "i=block/cogwheel";
        r1: Router;
            display: "i=abstract/router";
        r2: Router;
            display: "i=abstract/router";
        r3: Router;
            display: "i=abstract/router";
        cli: StandardHost[n];
            display: "i=device/laptop";
        srv: StandardHost;
            display: "i=device/server_l";
    connections nocheck:
        for i=0..n-1 do
            cli[i].out++ --> ethernetline --> r1.in++;
            cli[i].in++ <-- ethernetline <-- r1.out++;
        endfor

        r1.out++ --> ethernetline --> r2.in++;
        r1.in++ <-- ethernetline <-- r2.out++;

        r2.out++ --> ethernetline --> r3.in++;
        r2.in++ <-- ethernetline <-- r3.out++;

        r3.out++ --> ethernetline --> srv.in++;
        r3.in++ <-- ethernetline <-- srv.out++;

endmodule

network dllTest : DLLTest
endnetwork

