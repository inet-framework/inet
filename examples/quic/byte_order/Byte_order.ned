//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.quic.byte_order;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.common.scenario.ScenarioManager;

network Byte_order
{
   parameters:
        double linkDatarate @unit(bps) = default(1Gbps);
        double linkDelay @unit(s) = default(0ms);
        double linkPer = default(0);
        double bottleneckDatarate @unit(bps) = default(100Mbps);
        double bottleneckDelay @unit(s) = default(10ms);
        double bottleneckPer = default(0);
    types:
        channel link extends ned.DatarateChannel
        {
            datarate = parent.linkDatarate;
            delay = parent.linkDelay;
            per = parent.linkPer;
        }
        channel bottleneckLink extends ned.DatarateChannel
        {
            datarate = parent.bottleneckDatarate;
            delay = parent.bottleneckDelay;
            per = parent.bottleneckPer;
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
        sender.pppg++ <--> link <--> router1.pppg++;
        router2.pppg++ <--> link <--> receiver.pppg++;

        router1.pppg++ <--> bottleneckLink <--> router2.pppg++;
}
