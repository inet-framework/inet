//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.ethernet.arptest;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network ARPTest
{
    types:
        channel fiberline extends DatarateChannel
        {
            delay = 1us;
            datarate = 512Mbps;
        }
        channel ethline extends DatarateChannel
        {
            delay = 50ns;
            datarate = 100Mbps;
        }
    submodules:
        client: StandardHost {
            @display("p=250,100;i=device/laptop");
        }
        host1: StandardHost {
            @display("p=250,200;i=device/pc");
        }
        host2: StandardHost {
            @display("p=250,300;i=device/pc");
        }
        switch: EthernetSwitch {
            @display("p=400,200");
        }
        net: Router {
            @display("p=700,200");
        }
        router: Router {
            @display("p=550,200");
        }
        server: StandardHost {
            @display("p=850,200;i=device/server");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
    connections:
        client.ethg++ <--> ethline <--> switch.ethg++;
        switch.ethg++ <--> ethline <--> host1.ethg++;
        switch.ethg++ <--> ethline <--> host2.ethg++;
        router.ethg++ <--> ethline <--> switch.ethg++;
        router.pppg++ <--> fiberline <--> net.pppg++;
        server.pppg++ <--> fiberline <--> net.pppg++;
}

