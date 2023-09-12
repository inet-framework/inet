//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.examples.inet.tcpthroughput;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network ClientServer
{
    submodules:
        client1: StandardHost {
            @display("p=250,150;i=device/pc3");
        }
        server: StandardHost {
            @display("p=400,150;i=device/pc2");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
    connections:
        client1.pppg++ <--> { datarate = 10Mbps; } <--> server.pppg++;
}
