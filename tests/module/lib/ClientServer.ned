//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.test.moduletest.lib;

import ned.DatarateChannel;
import inet.node.inet.StandardHost;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;


network ClientServer
{
    parameters:
        double per = default(0);
        @display("bgb=232,193");
    types:
        channel C extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 0.1us;
            per = parent.per;
        }
    submodules:
        client: StandardHost {
            parameters:
                @display("p=53,67;i=device/pc3");
        }
        server: StandardHost {
            parameters:
                @display("p=181,67;i=device/pc2");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=53,134");
        }
    connections:
        client.pppg++ <--> C <--> server.pppg++;
}

