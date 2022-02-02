//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.examples.inet.tcp_ppp_reconnect;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network ClientServerWithSM
{
    types:
        channel C extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 0.1us;
        }
    submodules:
        client1: StandardHost {
            parameters:
                @display("p=250,200;i=device/pc3");
        }
        server: StandardHost {
            parameters:
                @display("p=400,200;i=device/pc2");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        scenarioManager: ScenarioManager {
            @display("p=100,300;is=s");
        }
    connections:
        client1.pppg++ <--> C <--> server.pppg++;
}

