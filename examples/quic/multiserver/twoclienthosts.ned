//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.quic.multiserver;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

network twoclienthosts
{
    parameters:

    types:
        channel etherline extends ned.DatarateChannel
        {
            delay = 1ms;
            datarate = 1000Mbps;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=222,32");
        }

        router1: Router {
            parameters:
                @display("p=146,137");
        }

        router2: Router {
            parameters:
                @display("p=305,137");
        }

        client1: StandardHost {
            parameters:
                @display("i=device/laptop;p=52,104");
        }

        client2: StandardHost {
            parameters:
                @display("i=device/laptop;p=52,170");
        }

        server: StandardHost {
            parameters:
                @display("p=390,137;i=device/server_l");
        }

    connections:
        client1.pppg++ <--> etherline <--> router1.pppg++;
        
        client2.pppg++ <--> etherline <--> router1.pppg++;

        router1.pppg++ <--> etherline <--> router2.pppg++;

        router2.pppg++ <--> etherline <--> server.pppg++;
}
