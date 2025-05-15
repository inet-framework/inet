//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package cutthroughswitching;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;

network CutthroughSwitchingShowcase
{
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100");
        }
        source: TsnDevice {
            @display("p=350,100");
        }
        switch1: TsnSwitch {
            @display("p=550,100");
        }
        switch2: TsnSwitch {
            @display("p=750,100");
        }
        destination: TsnDevice {
            @display("p=950,100");
        }
    connections:
        source.ethg++ <--> EthernetLink <--> switch1.ethg++;
        switch1.ethg++ <--> EthernetLink <--> switch2.ethg++;
        switch2.ethg++ <--> EthernetLink <--> destination.ethg++;
}

