//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.measurement.residencetime;

import inet.networks.base.WiredNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth1G;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;

network ResidenceTimeMeasurementShowcase extends WiredNetworkBase
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
        source.ethg++ <--> Eth1G <--> switch.ethg++;
        switch.ethg++ <--> Eth100M <--> destination.ethg++;
}

