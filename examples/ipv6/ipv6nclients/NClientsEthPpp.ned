//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//



package inet.examples.ipv6.ipv6nclients;

import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
import inet.node.ipv6.Router6;
import inet.node.ipv6.StandardHost6;
import ned.DatarateChannel;


network NClientsEthPpp
{
    parameters:
        int n;
    types:
        channel fiberline extends DatarateChannel
        {
            delay = 1us;
            datarate = 1Gbps;
        }
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        configurator: Ipv6FlatNetworkConfigurator {
            @display("p=100,100;is=s");
        }
        r1: Router6 {
            @display("p=400,100");
        }
        r2: Router6 {
            @display("p=550,100");
        }
        r3: Router6 {
            @display("p=700,100");
        }
        cli[n]: StandardHost6 {
            @display("p=250,100,col,100");
        }
        srv: StandardHost6 {
            @display("p=850,100");
        }
    connections:
        for i=0..n-1 {
            cli[i].ethg++ <--> ethernetline <--> r1.ethg++;
        }
        r1.pppg++ <--> fiberline <--> r2.pppg++;
        r2.ethg++ <--> ethernetline <--> r3.ethg++;
        r3.pppg++ <--> fiberline <--> srv.pppg++;
}

