//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.measurement.throughput;

import inet.networks.base.WiredNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.inet.StandardHost;

network ChannelThroughputMeasurementShowcase extends WiredNetworkBase
{
    submodules:
        source: StandardHost {
            @display("p=350,100");
        }
        destination: StandardHost {
            @display("p=550,100");
        }
    connections:
        source.ethg++ <--> EthernetLink <--> destination.ethg++;
}

