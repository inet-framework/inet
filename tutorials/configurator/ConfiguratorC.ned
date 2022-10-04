//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.tutorials.configurator;

import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth10M;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


channel Eth100MLong extends Eth100M
{
    length = 100m;
}
network ConfiguratorC extends ConfiguratorB
{
    submodules:
        area1AP: AccessPoint {
            @display("p=611.64716,928.3376");
        }
        area3AP: AccessPoint {
            @display("p=2552.152,881.76544");
        }
        area1lan3host1: WirelessHost {
            @display("p=211.3514,585.5005");
        }
        area1lan3host2: WirelessHost {
            @display("p=211.3514,754.0104");
        }
        area1lan3host3: WirelessHost {
            @display("p=211.3514,911.0958");
        }
        area3lan3host1: WirelessHost {
            @display("p=2825.3418,551.44696");
        }
        area3lan3host2: WirelessHost {
            @display("p=2936.0706,725.44934");
        }
        area3lan3host3: WirelessHost {
            @display("p=2824.6829,885.3909");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=150,400");
        }
    connections:
        area1router.ethg++ <--> Eth10M <--> area1AP.ethg++;
        area3router.ethg++ <--> Eth10M <--> area3AP.ethg++;
}
