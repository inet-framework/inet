//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

package inet.examples.diffserv.simple_;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


//
// This network contains a router with an 10Mbps Ethernet interface,
// and with a 128kbps dialup connection to a server.
//
// The dialup connection is the bottleneck of the traffic from the clients
// to the server. Without QoS, high traffic can cause random packet drops
// at the router PPP interface.
//
network DiffservNetwork
{
    parameters:
        int numClients = default(1);
    types:
        channel dialup extends DatarateChannel
        {
            delay = normal(0.004s, 0.0018s);
            datarate = 128kbps;
        }
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 10Mbps;
        }
    submodules:
        router: Router {
            @display("p=318,108");
        }
        client[numClients]: StandardHost;
        server: StandardHost {
            @display("p=436,108");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=181,21");
        }
        etherSwitch: EtherSwitch {
            @display("p=198,107");
        }
    connections:
        router.pppg++ <--> dialup <--> server.pppg++;
        etherSwitch.ethg++ <--> ethernetline <--> router.ethg++;
        for i=0..numClients-1 {
            client[i].ethg++ <--> ethernetline <--> etherSwitch.ethg++;
        }
}
