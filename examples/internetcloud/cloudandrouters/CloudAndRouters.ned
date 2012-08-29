//
// Copyright (C) 2012 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.examples.internetcloud.cloudandrouters;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.ethernet.Eth100M;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import inet.nodes.internetcloud.InternetCloud;
import ned.DatarateChannel;


network CloudAndRouters
{
    parameters:
        int numSenders;

    types:
        channel C extends DatarateChannel
        {
            delay = 20ms;
            datarate = 5Mbps;
        }
    submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=61,163");
        }

        srouter[numSenders]: Router;
        sender[numSenders]: StandardHost;
        rrouter: Router;
        recip: StandardHost;
        internet: InternetCloud;
    connections:
        rrouter.pppg++ <--> C <--> internet.pppg++;
        recip.ethg++ <--> Eth100M <--> rrouter.ethg++;
        for i=0..numSenders-1 {
            srouter[i].pppg++ <--> C <--> internet.pppg++;
            sender[i].ethg++ <--> Eth100M <--> srouter[i].ethg++;
        }
}

