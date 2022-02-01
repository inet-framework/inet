//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.tsn.cutthroughswitching;

import inet.networks.base.TsnNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;

network CutthroughSwitchingShowcase extends TsnNetworkBase
{
    submodules:
        device1: TsnDevice {
            @display("p=350,200");
        }
        switch1: TsnSwitch {
            @display("p=550,200");
        }
        switch2: TsnSwitch {
            @display("p=750,200");
        }
        device2: TsnDevice {
            @display("p=950,200");
        }
    connections:
        device1.ethg++ <--> EthernetLink <--> switch1.ethg++;
        switch1.ethg++ <--> EthernetLink <--> switch2.ethg++;
        switch2.ethg++ <--> EthernetLink <--> device2.ethg++;
}

