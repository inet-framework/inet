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

package inet.examples.voipstream.VoIPStreamTest;


import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.EtherSwitch;
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
        switch: EtherSwitch {
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
        configurator: IPv4NetworkConfigurator {
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
