//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.examples.inet.bulktransfer;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
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
                @display("p=250,100;i=device/pc3");
        }
        client2: StandardHost {
            parameters:
                @display("p=550,100;i=device/pc3");
        }
        client3: StandardHost {
            parameters:
                @display("p=550,300;i=device/pc3");
        }
        server: StandardHost {
            parameters:
                @display("p=250,300;i=device/pc2");
        }
        router: Router {
            parameters:
                @display("p=400,200");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
    connections:
        client1.pppg++ <--> C <--> router.pppg++;
        client2.pppg++ <--> C <--> router.pppg++;
        client3.pppg++ <--> C <--> router.pppg++;
        client1.pppg++ <--> C <--> server.pppg++;
        server.pppg++ <--> C <--> router.pppg++;
}

