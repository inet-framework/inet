
// ----------------------------------------------------------------------------
//
// HttpTools Project
//
// This file is a part of the HttpTools project. The project was created at
// Reykjavik University, the Laboratory for Dependable Secure Systems (LDSS).
// Its purpose is to create a set of OMNeT++ components to simulate browsing
// behaviour in a high-fidelity manner along with a highly configurable
// Web server component.
//
// Maintainer: Kristjan V. Jonsson (LDSS) kristjanvj@gmail.com
// Project home page: code.google.com/p/omnet-httptools
//
// ----------------------------------------------------------------------------
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
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
// ----------------------------------------------------------------------------

import
    "Router",
    "StandardHost",
    "FlatNetworkConfigurator",
    "NAMTrace",
    "httptController";

channel ethernetline
    delay 0.1us;
    datarate 100*1000000;
endchannel

//
// 10-nodes test scenario for sockets.
//
// The scenario consists of a number of clients and 10 servers connected to two routers. The routers are connected
// with a single link.
//
module httpt10servers
    parameters:
        numclients: numeric const;
    submodules:
        configurator: FlatNetworkConfigurator;
            parameters:
                moduleTypes = "Router StandardHost",
                nonIPModuleTypes = "",
                networkAddress = "145.236.0.0",
                netmask = "255.255.0.0";
            display: "p=58,79;i=block/cogwheel";
        controller: httptController;
            display: "p=62,156;i=block/cogwheel";
        router_c: Router;
            display: "p=492,424;i=abstract/router_l";
        router_s: Router;
            display: "p=492,320;i=abstract/router_l";
        client: StandardHost[numclients];
            display: "i=device/laptop_l";
        server1: StandardHost;
            display: "p=191,316;i=device/server_l";
        server2: StandardHost;
            display: "p=231,220;i=device/server_l";
        server3: StandardHost;
            display: "p=278,140;i=device/server_l";
        server4: StandardHost;
            display: "p=355,92;i=device/server_l";
        server5: StandardHost;
            display: "p=452,60;i=device/server_l";
        server6: StandardHost;
            display: "p=549,60;i=device/server_l";
        server7: StandardHost;
            display: "p=660,84;i=device/server_l";
        server8: StandardHost;
            display: "p=734,148;i=device/server_l";
        server9: StandardHost;
            display: "p=772,228;i=device/server_l";
        server10: StandardHost;
            display: "p=797,324;i=device/server_l";
    connections nocheck:
        server1.out++ --> ethernetline --> router_s.in++;
        server1.in++ <-- ethernetline <-- router_s.out++;
        server2.out++ --> ethernetline --> router_s.in++;
        server2.in++ <-- ethernetline <-- router_s.out++;
        server3.out++ --> ethernetline --> router_s.in++;
        server3.in++ <-- ethernetline <-- router_s.out++;
        server4.out++ --> ethernetline --> router_s.in++;
        server4.in++ <-- ethernetline <-- router_s.out++;
        server5.out++ --> ethernetline --> router_s.in++;
        server5.in++ <-- ethernetline <-- router_s.out++;
        server6.out++ --> ethernetline --> router_s.in++;
        server6.in++ <-- ethernetline <-- router_s.out++;
        server7.out++ --> ethernetline --> router_s.in++;
        server7.in++ <-- ethernetline <-- router_s.out++;
        server8.out++ --> ethernetline --> router_s.in++;
        server8.in++ <-- ethernetline <-- router_s.out++;
        server9.out++ --> ethernetline --> router_s.in++;
        server9.in++ <-- ethernetline <-- router_s.out++;
        server10.out++ --> ethernetline --> router_s.in++;
        server10.in++ <-- ethernetline <-- router_s.out++;
        for i=0..numclients-1 do
            client[i].out++ --> ethernetline --> router_c.in++;
            client[i].in++ <-- ethernetline <-- router_c.out++;
        endfor;
        router_s.out++ --> ethernetline --> router_c.in++;
        router_s.in++ <-- ethernetline <-- router_c.out++;

    display: "b=939,624";
endmodule

network httpt10servers : httpt10servers
    parameters:
        numclients = input(10, "Number of clients");
endnetwork

