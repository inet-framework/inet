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

import inet.applications.httptools.configurator.HttpController;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;


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
        int numclients @prompt("Number of clients") = default(10);
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                //                moduleTypes = "Router StandardHost";
                //                nonIPModuleTypes = "";
                @display("is=s;p=100,100;i=block/cogwheel");
        }
        httpController: HttpController {
            parameters:
                @display("is=s;p=100,200;i=block/cogwheel");
        }
        router_c: Router {
            parameters:
                @display("p=850,250;i=abstract/router");
        }
        router_s: Router {
            parameters:
                @display("p=400,250;i=abstract/router");
        }
        client[numclients]: StandardHost {
            parameters:
                @display("i=device/laptop;p=700,80,ri,150,170");
        }
        server1: StandardHost {
            parameters:
                @display("p=300,120;i=device/server");
        }
        server2: StandardHost {
            parameters:
                @display("p=230,200;i=device/server");
        }
        server3: StandardHost {
            parameters:
                @display("p=230,300;i=device/server");
        }
        server4: StandardHost {
            parameters:
                @display("p=300,370;i=device/server");
        }
        server5: StandardHost {
            parameters:
                @display("p=400,420;i=device/server");
        }
        server6: StandardHost {
            parameters:
                @display("p=500,370;i=device/server");
        }
        server7: StandardHost {
            parameters:
                @display("p=570,300;i=device/server");
        }
        server8: StandardHost {
            parameters:
                @display("p=570,200;i=device/server");
        }
        server9: StandardHost {
            parameters:
                @display("p=500,120;i=device/server");
        }
        server10: StandardHost {
            parameters:
                @display("p=400,80;i=device/server");
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

