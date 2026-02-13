//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//






package inet.examples.inet.flatnet;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network FlatNet
{
    types:
        channel fiberline extends DatarateChannel
        {
            delay = 1us;
            datarate = 512Mbps;
        }
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 10Mbps;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='*' address='192.168.x.x' netmask='255.255.x.x'/></config>");
                @display("p=100,100;is=s");
        }
        rte[20]: Router;
        cli[2]: StandardHost {
            parameters:
                @display("i=device/laptop");
        }
        srv: StandardHost {
            parameters:
                @display("i=device/server");
        }
    connections:
        cli[0].pppg++ <--> ethernetline <--> rte[16].pppg++;
        cli[1].pppg++ <--> ethernetline <--> rte[8].pppg++;
        srv.pppg++ <--> fiberline <--> rte[18].pppg++;
        rte[0].pppg++ <--> fiberline <--> rte[1].pppg++;
        rte[1].pppg++ <--> fiberline <--> rte[2].pppg++;
        rte[1].pppg++ <--> fiberline <--> rte[4].pppg++;
        rte[3].pppg++ <--> fiberline <--> rte[4].pppg++;
        rte[4].pppg++ <--> fiberline <--> rte[5].pppg++;
        rte[4].pppg++ <--> fiberline <--> rte[7].pppg++;
        rte[5].pppg++ <--> fiberline <--> rte[6].pppg++;
        rte[5].pppg++ <--> fiberline <--> rte[10].pppg++;
        rte[6].pppg++ <--> fiberline <--> rte[7].pppg++;
        rte[6].pppg++ <--> fiberline <--> rte[9].pppg++;
        rte[7].pppg++ <--> fiberline <--> rte[8].pppg++;
        rte[7].pppg++ <--> fiberline <--> rte[12].pppg++;
        rte[9].pppg++ <--> fiberline <--> rte[11].pppg++;
        rte[10].pppg++ <--> fiberline <--> rte[11].pppg++;
        rte[10].pppg++ <--> fiberline <--> rte[13].pppg++;
        rte[11].pppg++ <--> fiberline <--> rte[12].pppg++;
        rte[11].pppg++ <--> fiberline <--> rte[14].pppg++;
        rte[12].pppg++ <--> fiberline <--> rte[15].pppg++;
        rte[13].pppg++ <--> fiberline <--> rte[18].pppg++;
        rte[14].pppg++ <--> fiberline <--> rte[15].pppg++;
        rte[15].pppg++ <--> fiberline <--> rte[16].pppg++;
        rte[16].pppg++ <--> fiberline <--> rte[17].pppg++;
        rte[16].pppg++ <--> fiberline <--> rte[19].pppg++;
}


