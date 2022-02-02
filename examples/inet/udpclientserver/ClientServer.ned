//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.examples.inet.udpclientserver;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network UdpClientServer
{
    parameters:
        double per = default(0);
    types:
        channel C extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 0.1us;
            per = parent.per;
        }
    submodules:
        client1: StandardHost {
            parameters:
                @display("p=250,100;i=device/pc3");
        }
        server: StandardHost {
            parameters:
                @display("p=400,100;i=device/pc2");
        }
        unavserver: StandardHost {
            parameters:
                @display("p=250,200;i=device/pc2");
        }
        unavserver2: StandardHost {
            parameters:
                @display("p=400,200;i=device/pc2");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
//        configurator: NextHopNetworkConfigurator {
//            parameters:
//                @display("p=53,134");
//        }
        scenarioManager: ScenarioManager {
            @display("p=100,300;is=s");
        }
    connections allowunconnected:
        client1.pppg++ <--> C <--> server.pppg++;
        unavserver.pppg++ <--> C <--> unavserver2.pppg++;
}

