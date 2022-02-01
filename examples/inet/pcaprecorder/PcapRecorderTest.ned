//
// Copyright (C) 2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.inet.pcaprecorder;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network PcapRecorderTest
{
    types:
        channel C extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 0.1us;
        }
    submodules:
        client0: StandardHost {
            parameters:
                @display("p=250,100;i=device/pc3");
        }
        client1: StandardHost {
            parameters:
                @display("p=250,200;i=device/pc3");
        }
        server: StandardHost {
            parameters:
                @display("p=350,150;i=device/pc2");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
    connections:
        client0.pppg++ <--> C <--> server.pppg++;
        client1.ethg++ <--> C <--> server.ethg++;
}

