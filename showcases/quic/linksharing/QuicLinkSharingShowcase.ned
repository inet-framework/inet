//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.quic.linksharing;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

network QuicLinkSharingShowcase
{
    parameters:
        double bottleneckPer = default(0);
        double bottleneckDelay @unit(s) = default(1ms);
        double bottleneckDatarate @unit(bps) = default(100Mbps);

    types:
        channel C extends ned.DatarateChannel { datarate = 1Gbps; }
        channel bottleneck extends ned.DatarateChannel
        {
            datarate = parent.bottleneckDatarate;
            delay = parent.bottleneckDelay;
            per = parent.bottleneckPer;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator { @display("p=150,50"); }

        sender1: StandardHost { @display("p=50,50"); }
        sender2: StandardHost { @display("p=50,150"); }
        sender3: StandardHost { @display("p=50,250"); }
        backgroundSender: StandardHost { @display("p=100,300"); }

        router1: Router { @display("p=150,150"); }
        router2: Router { @display("p=300,150"); }

        receiver1: StandardHost { @display("p=400,50"); }
        receiver2: StandardHost { @display("p=400,150"); }
        receiver3: StandardHost { @display("p=400,250"); }
        backgroundReceiver: StandardHost { @display("p=350,300"); }

    connections:
        sender1.pppg++ <--> C <--> router1.pppg++;
        sender2.pppg++ <--> C <--> router1.pppg++;
        sender3.pppg++ <--> C <--> router1.pppg++;
        backgroundSender.pppg++ <--> C <--> router1.pppg++;

        router1.pppg++ <--> bottleneck <--> router2.pppg++;

        router2.pppg++ <--> C <--> receiver1.pppg++;
        router2.pppg++ <--> C <--> receiver2.pppg++;
        router2.pppg++ <--> C <--> receiver3.pppg++;
        router2.pppg++ <--> C <--> backgroundReceiver.pppg++;
}
