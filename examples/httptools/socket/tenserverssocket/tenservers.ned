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


package inet.examples.httptools.socket.tenserverssocket;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import inet.world.httptools.HttpController;


channel ethernetline extends ned.DatarateChannel
{
    parameters:
        delay = 0.1us;
        datarate = 100Mbps;
}

//
// 10-nodes test scenario for sockets.
//
// The scenario consists of a number of clients and 10 servers connected to two routers. The routers are connected
// with a single link.
//
network Http10Servers
{
    parameters:
        double numclients @prompt("Number of clients") = default(10);
        @display("bgb=939,624");
    submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                //                moduleTypes = "Router StandardHost";
                //                nonIPModuleTypes = "";
                @display("p=58,79;i=block/cogwheel");
        }
        controller: HttpController {
            parameters:
                @display("p=62,156;i=block/cogwheel");
        }
        router_c: Router {
            parameters:
                @display("p=492,424;i=abstract/router_l");
        }
        router_s: Router {
            parameters:
                @display("p=492,320;i=abstract/router_l");
        }
        client[numclients]: StandardHost {
            parameters:
                @display("i=device/laptop_l");
        }
        server1: StandardHost {
            parameters:
                @display("p=191,316;i=device/server_l");
        }
        server2: StandardHost {
            parameters:
                @display("p=231,220;i=device/server_l");
        }
        server3: StandardHost {
            parameters:
                @display("p=278,140;i=device/server_l");
        }
        server4: StandardHost {
            parameters:
                @display("p=355,92;i=device/server_l");
        }
        server5: StandardHost {
            parameters:
                @display("p=452,60;i=device/server_l");
        }
        server6: StandardHost {
            parameters:
                @display("p=549,60;i=device/server_l");
        }
        server7: StandardHost {
            parameters:
                @display("p=660,84;i=device/server_l");
        }
        server8: StandardHost {
            parameters:
                @display("p=734,148;i=device/server_l");
        }
        server9: StandardHost {
            parameters:
                @display("p=772,228;i=device/server_l");
        }
        server10: StandardHost {
            parameters:
                @display("p=797,324;i=device/server_l");
        }
    connections allowunconnected:
        server1.ethg++ <--> ethernetline <--> router_s.ethg++;
        server2.ethg++ <--> ethernetline <--> router_s.ethg++;
        server3.ethg++ <--> ethernetline <--> router_s.ethg++;
        server4.ethg++ <--> ethernetline <--> router_s.ethg++;
        server5.ethg++ <--> ethernetline <--> router_s.ethg++;
        server6.ethg++ <--> ethernetline <--> router_s.ethg++;
        server7.ethg++ <--> ethernetline <--> router_s.ethg++;
        server8.ethg++ <--> ethernetline <--> router_s.ethg++;
        server9.ethg++ <--> ethernetline <--> router_s.ethg++;
        server10.ethg++ <--> ethernetline <--> router_s.ethg++;
        for i=0..numclients-1 {
            client[i].ethg++ <--> ethernetline <--> router_c.ethg++;
        }
        router_s.ethg++ <--> ethernetline <--> router_c.ethg++;

}

