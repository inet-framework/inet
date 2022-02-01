//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.timing;

import inet.networks.base.WiredNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;

network TimingExample extends WiredNetworkBase
{
    submodules:
        client1: StandardHost {
            @display("p=300,150");
        }
        client2: StandardHost {
            @display("p=300,350");
        }
        switch1: EthernetSwitch {
            @display("p=500,250");
        }
        switch2: EthernetSwitch {
            @display("p=700,250");
        }
        server1: StandardHost {
            @display("p=900,150");
        }
        server2: StandardHost {
            @display("p=900,350");
        }
    connections:
        client1.ethg++ <--> Eth100M <--> switch1.ethg++;
        client2.ethg++ <--> Eth100M <--> switch1.ethg++;
        switch2.ethg++ <--> Eth100M <--> server1.ethg++;
        switch2.ethg++ <--> Eth100M <--> server2.ethg++;
        switch1.ethg++ <--> Eth100M <--> switch2.ethg++;
}
