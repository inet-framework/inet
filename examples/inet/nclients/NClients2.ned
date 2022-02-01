//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.inet.nclients;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network NClients2
{
    parameters:
        int numRouters;
        int hostsPerRouter;
    types:
        channel ethernetline2 extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
        channel gigabitline2 extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 1Gbps;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        r[numRouters]: Router;
        cli[numRouters*hostsPerRouter]: StandardHost {
            parameters:
                @display("i=device/laptop");
        }
        srv: StandardHost {
            parameters:
                @display("i=device/server;p=250,100");
        }
    connections:
        for i=0..numRouters-1, for j=0..hostsPerRouter-1 {
            cli[i*hostsPerRouter+j].pppg++ <--> ethernetline2 <--> r[i].pppg++;
        }
        for i=0..numRouters-2 {
            r[i].pppg++ <--> gigabitline2 <--> r[i+1].pppg++;
        }
        r[numRouters-1].pppg++ <--> ethernetline2 <--> srv.pppg++;
}

