//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.measurement.endtoenddelay;

import inet.networks.base.WiredNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;

network EndToEndDelayMeasurementShowcase extends WiredNetworkBase
{
    submodules:
        source: StandardHost {
            @display("p=350,100");
        }
        destination: StandardHost {
            @display("p=550,100");
        }
    connections:
        source.ethg++ <--> Eth100M <--> destination.ethg++;
}

