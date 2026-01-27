//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.quic.dplpmtud;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.common.scenario.ScenarioManager;

network Dplpmtud
{
    parameters:

        @display("bgb=453,279");
    types:
        channel etherline extends ned.DatarateChannel
        {
            delay = 1ms;
            datarate = 1000Mbps;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=222,32");
            	config = xmldoc("config.xml");
        }

        router1: Router {
            parameters:
                @display("p=146,137");
        }

        router2: Router {
            parameters:
                @display("p=305,137");
        }

        router3: Router {
            parameters:
                @display("p=222,227");
        }

        sender: StandardHost {
            parameters:
                @display("i=device/laptop;p=57,137");
        }

        receiver: StandardHost {
            parameters:
                @display("p=390,137;i=device/server_l");
        }

        scenarioManager: ScenarioManager {
            @display("p=103,24;is=s");
        }

    connections:
        sender.pppg++ <--> etherline <--> router1.pppg++;

        router1.pppg++ <--> etherline <--> router2.pppg++;

        router2.pppg++ <--> etherline <--> receiver.pppg++;

        router1.pppg++ <--> etherline <--> router3.pppg++;

        router3.pppg++ <--> etherline <--> router2.pppg++;
}
