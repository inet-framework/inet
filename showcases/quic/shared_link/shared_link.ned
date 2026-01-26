//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.quic.shared_link;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

network shared_link
{
    parameters:
        double bottleneckPer = default(0);
        double bottleneckDelay @unit(s) = default(1ms);
        double bottleneckDatarate @unit(bps) = default(100Mbps);

    types:
        channel C extends ned.DatarateChannel
        {
            datarate = 1Gbps;
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

        router1: Router {
            parameters:
                @display("p=146,137");
        }

        router2: Router {
            parameters:
                @display("p=305,137");
        }

        sender1: StandardHost {
            parameters:
                @display("p=57,62");
        }

        sender2: StandardHost {
            parameters:
                @display("p=57,137");
        }

        sender3: StandardHost {
            parameters:
                @display("p=57,187");
        }

        backgroundSender: StandardHost {
            parameters:
                @display("p=57,212");
        }

        receiver1: StandardHost {
            parameters:
                @display("p=390,62");
        }

        receiver2: StandardHost {
            parameters:
                @display("p=390,137");
        }

        receiver3: StandardHost {
            parameters:
                @display("p=390,187");
        }

        backgroundReceiver: StandardHost {
            parameters:
                @display("p=390,212");
        }

    connections:
        sender1.pppg++ <--> C <--> router1.pppg++;
        sender2.pppg++ <--> C <--> router1.pppg++;
        sender3.pppg++ <--> C <--> router1.pppg++;
        backgroundSender.pppg++ <--> C <--> router1.pppg++;

        router2.pppg++ <--> C <--> receiver1.pppg++;
        router2.pppg++ <--> C <--> receiver2.pppg++;
        router2.pppg++ <--> C <--> receiver3.pppg++;
        router2.pppg++ <--> C <--> backgroundReceiver.pppg++;

        router1.pppg++ <--> bottleneck <--> router2.pppg++;

}
