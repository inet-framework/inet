//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.tsn.framepreemption;

import inet.networks.base.WiredNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;

network FramePreemptionShowcase extends WiredNetworkBase
{
    submodules:
        host1: StandardHost {
            @display("p=250,100");
        }
        host2: StandardHost {
            @display("p=350,100");
        }
    connections:
        host1.ethg++ <--> Eth100M <--> host2.ethg++;
}

