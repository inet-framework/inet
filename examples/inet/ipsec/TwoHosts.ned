//
// Copyright (C) 2020
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

package inet.examples.inet.ipsec;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network TwoHosts
{
    submodules:
        client1: StandardHost {
            @display("p=50,60;i=device/pc3");
        }
        server: StandardHost {
            @display("p=300,60;i=device/pc2");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=53,134");
        }
    connections:
        client1.pppg++ <--> { datarate = 10Mbps; delay = 0.1us; } <--> server.pppg++;
}

