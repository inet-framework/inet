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




package inet.examples.inet.bulktransfer;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network BulkTransfer
{
    types:
        channel C extends DatarateChannel
        {
            datarate = 1Mbps;
            delay = 0.1us;
        }
    submodules:
        client1: StandardHost {
            parameters:
                @display("p=131,67;i=device/pc3");
        }
        client2: StandardHost {
            parameters:
                @display("p=337,67;i=device/pc3");
        }
        client3: StandardHost {
            parameters:
                @display("p=337,247;i=device/pc3");
        }
        server: StandardHost {
            parameters:
                @display("p=131,247;i=device/pc2");
        }
        router: Router {
            parameters:
                @display("p=239,156");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=62,119");
        }
    connections:
        client1.pppg++ <--> C <--> router.pppg++;
        client2.pppg++ <--> C <--> router.pppg++;
        client3.pppg++ <--> C <--> router.pppg++;
        client1.pppg++ <--> C <--> server.pppg++;
        server.pppg++ <--> C <--> router.pppg++;
}

