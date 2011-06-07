
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
    "ChannelInstaller",
    "httptController";

channel ethernetline
    delay 0.1us;
    datarate 100*1000000;
endchannel

//
// n-nodes test scenario for sockets. 
//
// The scenario consists of a number of clients and servers connected to two routers. The routers are connected
// with a single link.
//
module httptNnodes
    parameters:
        numservers: numeric const,
        numclients: numeric const;
    submodules:
        channelInstaller: ChannelInstaller;
            parameters:
                channelClass = "ThruputMeteringChannel",
                channelAttrs = "format=u";
            display: "p=98,50;i=block/cogwheel_s";
        configurator: FlatNetworkConfigurator;
            parameters:
                moduleTypes = "Router StandardHost",
                nonIPModuleTypes = "",
                networkAddress = "145.236.0.0",
                netmask = "255.255.0.0";
            display: "p=50,64;i=block/cogwheel";
        controller: httptController;
            display: "p=46,140;i=block/cogwheel";
        router_c: Router;
            display: "p=100,200;i=abstract/router_l";
        router_s: Router;
            display: "p=300,200;i=abstract/router_l";
        client: StandardHost[numclients];
            display: "i=device/laptop_l";
		server: StandardHost[numservers];
            display: "i=device/server_l";
    connections nocheck:
        for i=0..numservers-1 do
            server[i].out++ --> ethernetline --> router_s.in++;
            server[i].in++ <-- ethernetline <-- router_s.out++;
        endfor
        for i=0..numclients-1 do
            client[i].out++ --> ethernetline --> router_c.in++;
            client[i].in++ <-- ethernetline <-- router_c.out++;
        endfor
		router_s.out++ --> ethernetline --> router_c.in++;
		router_s.in++ <-- ethernetline <-- router_c.out++;

    display: "b=474,374";
endmodule

network httptNnodes : httptNnodes
   parameters:
        numclients = input(10, "Number of clients"),
        numservers = input(10, "Number of servers");
endnetwork

