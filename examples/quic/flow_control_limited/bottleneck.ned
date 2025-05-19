//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.quic.flow_control_limited;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

network bottleneck
{
    parameters:
        double linkDelay @unit(s) = default(0ms);

    types:
        channel link extends ned.DatarateChannel
        {
            datarate = 100Gbps;
            delay = parent.linkDelay;
            per = 0;
        }
        channel idealLink extends ned.DatarateChannel
        {
            datarate = 1000Gbps;
            delay = 0ms;
            per = 0;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=222,32");
        }

        sender: StandardHost {
            parameters:
                @display("p=57,137");
        }

        router1: Router {
            parameters:
                @display("p=146,137");
        }

        router2: Router {
            parameters:
                @display("p=305,137");
        }

        receiver: StandardHost {
            parameters:
                @display("p=390,137");
        }

    connections:
        sender.pppg++ <--> idealLink <--> router1.pppg++;

        router2.pppg++ <--> idealLink <--> receiver.pppg++;

        router1.pppg++ <--> link <--> router2.pppg++;
}
