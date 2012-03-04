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


package inet.examples.httptools.socket.simpleddos;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import inet.world.httptools.HttpController;


//
// n-nodes test scenario for sockets.
//
// The scenario consists of a number of clients and servers connected to two routers. The routers are connected
// with a single link.
//
network HttpNnodes
{
    parameters:
        double numclients @prompt("Number of clients") = default(10);
        @display("bgb=474,374");
    types:
        channel ethernetline extends ned.DatarateChannel {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        //        channelInstaller: ChannelInstaller {
        //            parameters:
        ////                channelClass = "ThruputMeteringChannel";
        ////                channelAttrs = "format=u";
        //                @display("p=98,50;i=block/cogwheel_s");
        //        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                //                moduleTypes = "Router StandardHost";
                //                nonIPModuleTypes = "";
                config=xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.x.x'/></config>");
                @display("p=50,64;i=block/cogwheel");
        }
        controller: HttpController {
            parameters:
                @display("p=46,140;i=block/cogwheel");
        }
        router_c: Router {
            parameters:
                @display("p=100,200;i=abstract/router_l");
        }
        router_s: Router {
            parameters:
                @display("p=300,200;i=abstract/router_l");
        }
        client[numclients]: StandardHost {
            parameters:
                @display("i=device/laptop_l");
        }
        thegood: StandardHost {
            parameters:
                @display("i=device/server_l");
        }
        thebad: StandardHost {
            parameters:
                @display("i=device/server_l");
        }
        theugly: StandardHost {
            parameters:
                @display("i=device/server_l");
        }
    connections allowunconnected:
        thegood.ethg++ <--> ethernetline <--> router_s.ethg++;
        thebad.ethg++ <--> ethernetline <--> router_s.ethg++;
        theugly.ethg++ <--> ethernetline <--> router_s.ethg++;
        for i=0..numclients-1 {
            client[i].ethg++ <--> ethernetline <--> router_c.ethg++;
        }
        router_s.ethg++ <--> ethernetline <--> router_c.ethg++;
}

