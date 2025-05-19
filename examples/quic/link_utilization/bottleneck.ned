//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.quic.link_utilization;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

network bottleneck
{
    parameters:
        double bottleneckPer = default(0);
        double bottleneckDelay @unit(s) = default(1ms);
        double bottleneckDatarate @unit(bps) = default(100Mbps);

    types:
        channel etherline extends ned.DatarateChannel
        {
            delay = 0ms;
            datarate = 1000Gbps;
        }
        channel bottleneck extends ned.DatarateChannel
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
        sender.pppg++ <--> etherline <--> router1.pppg++;

        router1.pppg++ <--> bottleneck <--> router2.pppg++;

        router2.pppg++ <--> etherline <--> receiver.pppg++;
}