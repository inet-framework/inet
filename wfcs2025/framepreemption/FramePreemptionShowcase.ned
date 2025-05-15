//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package framepreemption;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.tsn.TsnDevice;

network FramePreemptionShowcase
{
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100");
        }
        source: TsnDevice {
            @display("p=250,100");
        }
        destination: TsnDevice {
            @display("p=350,100");
        }
    connections:
        source.ethg++ <--> Eth100M <--> destination.ethg++;
}

