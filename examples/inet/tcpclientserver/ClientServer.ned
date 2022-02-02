//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.examples.inet.tcpclientserver;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network ClientServer
{
    parameters:
        double per = default(0);
    types:
        channel C extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 0.1us;
            per = parent.per;
        }
    submodules:
        client1: StandardHost {
            parameters:
                @display("p=250,150;i=device/pc3");
        }
        server: StandardHost {
            parameters:
                @display("p=400,150;i=device/pc2");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
    connections:
        client1.pppg++ <--> C <--> server.pppg++;
}

