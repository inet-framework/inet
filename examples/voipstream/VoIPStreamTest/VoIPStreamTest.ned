//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.voipstream.VoIPStreamTest;


import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;


network VoIPStreamTest
{
    types:
        channel line1 extends ned.DatarateChannel
        {
            parameters:
                delay = 100us;
                datarate = 100Mbps;
        }

        channel line2 extends ned.DatarateChannel
        {
            parameters:
                delay = 100us;
                datarate = 100Mbps;
        }
    submodules:
        client: StandardHost {
            parameters:
                @display("p=71,64;i=device/laptop_l");
        }
        switch: EthernetSwitch {
            parameters:
                @display("p=202,156;i=device/switch");
        }
        net: Router {
            parameters:
                @display("p=394,166;i=misc/cloud_l");
        }
        router: Router {
            parameters:
                @display("p=311,74;i=abstract/router");
        }
        server: StandardHost {
            parameters:
                @display("p=512,58;i=device/server_l");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='*' address='10.x.x.x' netmask='255.x.x.x'/></config>");
                @display("p=495,160;i=block/cogwheel_s");
        }
    connections:
        client.ethg++ <--> line1 <--> switch.ethg++;
        router.ethg++ <--> line1 <--> switch.ethg++;
        router.ethg++ <--> line2 <--> net.ethg++;
        server.ethg++ <--> line2 <--> net.ethg++;
}
