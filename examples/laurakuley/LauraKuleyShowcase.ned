//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.laurakuley;

import inet.networks.base.WiredNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;

network LauraKuleyShowcase extends WiredNetworkBase
{
    submodules:
        switch1: EthernetSwitch {
            @display("p=550,300");
        }
        switch2: EthernetSwitch {
            @display("p=750,300");
        }
        device1: StandardHost {
            @display("p=350,300");
        }
        device2: StandardHost {
            @display("p=550,100");
        }
        device3: StandardHost {
            @display("p=550,500");
        }
        device4: StandardHost {
            @display("p=750,100");
        }
        device5: StandardHost {
            @display("p=750,500");
        }
        device6: StandardHost {
            @display("p=950,300");
        }
    connections:
        device1.ethg++ <--> EthernetLink <--> switch1.ethg++;
        device2.ethg++ <--> EthernetLink <--> switch1.ethg++;
        device3.ethg++ <--> EthernetLink <--> switch1.ethg++;
        device4.ethg++ <--> EthernetLink <--> switch2.ethg++;
        device5.ethg++ <--> EthernetLink <--> switch2.ethg++;
        device6.ethg++ <--> EthernetLink <--> switch2.ethg++;
        switch1.ethg++ <--> EthernetLink <--> switch2.ethg++;
}

