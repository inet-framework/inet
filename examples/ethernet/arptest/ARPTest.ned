//
// Copyright (C) 2006 Andras Varga
//
// This library is free software, you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


package inet.examples.ethernet.arptest;

import ned.DatarateChannel;
import inet.nodes.inet.StandardHost;
import inet.nodes.inet.Router;
import inet.nodes.ethernet.EtherSwitch;
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;


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
            @display("p=71,64;i=device/laptop_l");
        }
        host1: StandardHost {
            @display("p=65,131;i=device/pc");
        }
        host2: StandardHost {
            @display("p=60,191;i=device/pc");
        }
        switch: EtherSwitch {
            @display("p=202,156");
        }
        net: Router {
            @display("p=394,166");
        }
        router: Router {
            @display("p=311,74");
        }
        server: StandardHost {
            @display("p=512,58;i=device/server_l");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=495,160");
        }
    connections:
        client.ethg++ <--> ethline <--> switch.ethg++;
        switch.ethg++ <--> ethline <--> host1.ethg++;
        switch.ethg++ <--> ethline <--> host2.ethg++;
        router.ethg++ <--> ethline <--> switch.ethg++;
        router.pppg++ <--> fiberline <--> net.pppg++;
        server.pppg++ <--> fiberline <--> net.pppg++;
}

