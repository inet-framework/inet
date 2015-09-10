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





package inet.examples.ipv6.ipv6bulk;

import inet.networklayer.configurator.ipv6.FlatNetworkConfigurator6;
import inet.node.ipv6.Router6;
import inet.node.ipv6.StandardHost6;
import ned.DatarateChannel;


network BulkTransfer6
{
    types:
        channel C extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 0.1us;
        }
    submodules:
        client1: StandardHost6 {
            parameters:
                @display("p=131,67;i=device/pc3");
        }
        client2: StandardHost6 {
            parameters:
                @display("p=337,67;i=device/pc3");
        }
        client3: StandardHost6 {
            parameters:
                @display("p=337,247;i=device/pc3");
        }
        server: StandardHost6 {
            parameters:
                @display("p=131,247;i=device/pc2");
        }
        router: Router6 {
            parameters:
                @display("p=239,156");
        }
        configurator: FlatNetworkConfigurator6 {
            parameters:
                @display("p=62,119");
        }
    connections:
        client1.ethg++ <--> C <--> router.ethg++;
        client2.ethg++ <--> C <--> router.ethg++;
        client3.ethg++ <--> C <--> router.ethg++;
        client1.ethg++ <--> C <--> server.ethg++;
        server.ethg++ <--> C <--> router.ethg++;
}

