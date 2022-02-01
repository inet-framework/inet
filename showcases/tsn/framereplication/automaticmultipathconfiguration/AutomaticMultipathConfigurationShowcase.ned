//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.tsn.framereplication.automaticmultipathconfiguration;

import inet.networks.base.TsnNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;

module LocalTsnSwitch extends TsnSwitch
{
    @defaultStatistic("gateStateChanged:vector"; module="eth[0].macLayer.queue.gate[0]");
}

network AutomaticMultipathConfigurationShowcase extends TsnNetworkBase
{
    submodules:
        source: TsnDevice {
            @display("p=300,200");
        }
        s1: LocalTsnSwitch {
            @display("p=500,200");
        }
        s2a: LocalTsnSwitch {
            @display("p=700,100");
        }
        s3a: LocalTsnSwitch {
            @display("p=900,100");
        }
        s2b: LocalTsnSwitch {
            @display("p=700,300");
        }
        s3b: LocalTsnSwitch {
            @display("p=900,300");
        }
        destination: TsnDevice {
            @display("p=1100,200");
        }
    connections:
        s3a.ethg++ <--> EthernetLink <--> destination.ethg++;
        s3b.ethg++ <--> EthernetLink <--> destination.ethg++;
        s2a.ethg++ <--> EthernetLink <--> s3a.ethg++;
        s2b.ethg++ <--> EthernetLink <--> s3b.ethg++;
        s2a.ethg++ <--> EthernetLink <--> s2b.ethg++;
        s1.ethg++ <--> EthernetLink <--> s2a.ethg++;
        s1.ethg++ <--> EthernetLink <--> s2b.ethg++;
        source.ethg++ <--> EthernetLink <--> s1.ethg++;
}

