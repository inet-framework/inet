//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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


package inet.examples.inet.multicast;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network MulticastNetwork
{
    types:
        channel pppline extends DatarateChannel
        {
            delay = normal(0.00015s, 0.00005s);
            datarate = 10Mbps;
        }
        channel dialup extends DatarateChannel
        {
            delay = normal(0.004s, 0.0018s);
            datarate = 64kbps;
        }
        channel fiberrouter extends DatarateChannel
        {
            delay = normal(0.000001s, 0.0000001s);
            datarate = 512Mbps;
        }
    submodules:
        host1: StandardHost {
            parameters:
                forwarding = false;
                @display("p=250,100;i=device/pc2");
            gates:
                pppg[1];
        }
        host2: StandardHost {
            parameters:
                forwarding = true;
                multicastForwarding = true;
                @display("p=150,300;i=device/pc2");
            gates:
                pppg[2];
        }
        host3: StandardHost {
            parameters:
                forwarding = true;
                multicastForwarding = true;
                @display("p=350,300;i=device/pc2");
            gates:
                pppg[2];
        }
        host4: StandardHost {
            parameters:
                forwarding = false;
                @display("p=600,350;i=device/pc2");
            gates:
                pppg[1];
        }
        host5: StandardHost {
            parameters:
                forwarding = false;
                @display("p=750,150;i=device/pc2");
            gates:
                pppg[1];
        }
        host6: StandardHost {
            parameters:
                forwarding = false;
                @display("p=750,250;i=device/pc2");
            gates:
                pppg[1];
        }
        router1: Router {
            parameters:
                multicastForwarding = true;
                @display("p=250,200");
            gates:
                pppg[4];
        }
        router2: Router {
            parameters:
                multicastForwarding = true;
                @display("p=500,200");
            gates:
                pppg[3];
        }
        router3: Router {
            parameters:
                multicastForwarding = true;
                @display("p=500,300");
            gates:
                pppg[2];
        }
        router4: Router {
            parameters:
                multicastForwarding = true;
                @display("p=650,200");
            gates:
                pppg[3];
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
    connections:
        // * Domain 1 * (172.0.0.x)
        host1.pppg[0] <--> pppline <--> router1.pppg[0];
        host2.pppg[0] <--> pppline <--> router1.pppg[1];
        host2.pppg[1] <--> dialup <--> host3.pppg[1];
        host3.pppg[0] <--> pppline <--> router1.pppg[2];

        // * Domain 2 * (172.0.1.x)
        host4.pppg[0] <--> pppline <--> router3.pppg[0];

        // * Domain 3 * (172.0.2.x)
        host5.pppg[0] <--> dialup <--> router4.pppg[0];
        host6.pppg[0] <--> dialup <--> router4.pppg[1];

        // * Router Connections * (172.1.0.x)
        router2.pppg[0] <--> fiberrouter <--> router1.pppg[3];
        router2.pppg[1] <--> fiberrouter <--> router3.pppg[1];
        router2.pppg[2] <--> fiberrouter <--> router4.pppg[2];
}
