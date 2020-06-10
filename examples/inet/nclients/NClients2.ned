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

package inet.examples.inet.nclients;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network NClients2
{
    parameters:
        int numRouters;
        int hostsPerRouter;
    types:
        channel pppline2 extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
        channel gigabitline2 extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 1Gbps;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        r[numRouters]: Router;
        cli[numRouters*hostsPerRouter]: StandardHost {
            parameters:
                @display("i=device/laptop");
        }
        srv: StandardHost {
            parameters:
                @display("i=device/server;p=250,100");
        }
    connections:
        for i=0..numRouters-1, for j=0..hostsPerRouter-1 {
            cli[i*hostsPerRouter+j].pppg++ <--> pppline2 <--> r[i].pppg++;
        }
        for i=0..numRouters-2 {
            r[i].pppg++ <--> gigabitline2 <--> r[i+1].pppg++;
        }
        r[numRouters-1].pppg++ <--> pppline2 <--> srv.pppg++;
}

