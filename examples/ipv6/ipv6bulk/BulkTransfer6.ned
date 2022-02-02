//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.examples.ipv6.ipv6bulk;

import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
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
                @display("p=250,100;i=device/pc3");
        }
        client2: StandardHost6 {
            parameters:
                @display("p=550,100;i=device/pc3");
        }
        client3: StandardHost6 {
            parameters:
                @display("p=550,300;i=device/pc3");
        }
        server: StandardHost6 {
            parameters:
                @display("p=250,300;i=device/pc2");
        }
        router: Router6 {
            parameters:
                @display("p=400,200");
        }
        configurator: Ipv6FlatNetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
    connections:
        client1.ethg++ <--> C <--> router.ethg++;
        client2.ethg++ <--> C <--> router.ethg++;
        client3.ethg++ <--> C <--> router.ethg++;
        client1.ethg++ <--> C <--> server.ethg++;
        server.ethg++ <--> C <--> router.ethg++;
}

