//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.quic.persistent_congestion;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.common.scenario.ScenarioManager;

network Persistent_congestion
{
    parameters:

        @display("bgb=451,190");
    types:
        channel ideal extends ned.DatarateChannel
        {
            delay = 1ms;
            datarate = 100Gbps;
        }
        channel bottleneck extends ned.DatarateChannel
        {
            delay = 10ms;
            datarate = 100Mbps;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=222,32");
        }

        router1: Router {
            parameters:
                @display("p=146,137");
        }

        router2: Router {
            parameters:
                @display("p=305,137");
        }

        sender: StandardHost {
            parameters:
                @display("p=57,137");
        }


        receiver: StandardHost {
            parameters:
                @display("p=390,137");
        }

        scenarioManager: ScenarioManager {
            @display("p=103,24;is=s");
        }

    connections:
        sender.pppg++ <--> ideal <--> router1.pppg++;

        router1.pppg++ <--> bottleneck <--> router2.pppg++;

        router2.pppg++ <--> ideal <--> receiver.pppg++;
}
