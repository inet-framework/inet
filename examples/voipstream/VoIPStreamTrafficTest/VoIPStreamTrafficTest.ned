//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.voipstream.VoIPStreamTrafficTest;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network VoIPStreamTrafficTest
{
    @display("bgb=750,447");
    types:
        channel line1 extends DatarateChannel
        {
            parameters:
                delay = 100us;
                datarate = 100Mbps;
        }

        channel line2 extends DatarateChannel
        {
            parameters:
                delay = 10us;
                datarate = 100Mbps;
        }
    submodules:
        client: StandardHost {
            parameters:
                @display("p=54,133;i=device/laptop_l");
        }
        switch: EthernetSwitch {
            parameters:
                @display("p=190,224;i=device/switch");
        }
        router2: Router {
            parameters:
                @display("p=376,297;i=abstract/router");
        }
        router1: Router {
            parameters:
                @display("p=266,144;i=abstract/router");
        }
        server: StandardHost {
            parameters:
                @display("p=589,58;i=device/server_l");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='*' address='10.0.x.x' netmask='255.255.x.x'/></config>");
                @display("p=44,19;i=block/cogwheel_s");
        }
        host1: StandardHost {
            @display("p=54,310");
        }
        host2: StandardHost {
            @display("p=589,332");
        }
        switch2: EthernetSwitch {
            @display("p=473,224");
        }
    connections:
        client.ethg++ <--> line1 <--> switch.ethg++;
        router1.ethg++ <--> line1 <--> switch.ethg++;
        router1.ethg++ <--> line2 <--> router2.ethg++;
        switch2.ethg++ <--> line2 <--> router2.ethg++;
        switch2.ethg++ <--> line1 <--> server.ethg++;
        switch2.ethg++ <--> line1 <--> host2.ethg++;
        switch.ethg++ <--> line1 <--> host1.ethg++;
}
