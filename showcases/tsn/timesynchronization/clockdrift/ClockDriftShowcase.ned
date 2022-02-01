//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.tsn.timesynchronization.clockdrift;

import inet.networks.base.WiredNetworkBase;
import inet.node.ethernet.Eth1G;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;

network ClockDriftShowcase extends WiredNetworkBase
{
    submodules:
        source1: StandardHost {
            @display("p=300,100");
        }
        source2: StandardHost {
            @display("p=300,300");
        }
        switch1: EthernetSwitch {
            @display("p=500,200");
        }
        switch2: EthernetSwitch {
            @display("p=700,200");
        }
        sink1: StandardHost {
            @display("p=900,100");
        }
        sink2: StandardHost {
            @display("p=900,300");
        }
    connections:
        switch1.ethg++ <--> Eth1G <--> switch2.ethg++;
        source1.ethg++ <--> Eth1G <--> switch1.ethg++;
        source2.ethg++ <--> Eth1G <--> switch1.ethg++;
        switch2.ethg++ <--> Eth1G <--> sink1.ethg++;
        switch2.ethg++ <--> Eth1G <--> sink2.ethg++;
}
