//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.measurement.datarate;

import inet.networks.base.WiredNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;

network DataRateMeasurementShowcase extends WiredNetworkBase
{
    submodules:
        source: StandardHost {
            @display("p=350,100");
        }
        switch: EthernetSwitch {
            @display("p=550,100");
        }
        destination: StandardHost {
            @display("p=750,100");
        }
    connections:
        source.ethg++ <--> EthernetLink <--> switch.ethg++;
        switch.ethg++ <--> EthernetLink <--> destination.ethg++;
}

