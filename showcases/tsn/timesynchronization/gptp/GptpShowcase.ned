//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.tsn.timesynchronization.gptp;

import inet.common.scenario.ScenarioManager;
import inet.networks.base.TsnNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnClock;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;

network OneMasterClockGptpShowcase extends TsnNetworkBase
{
    submodules:
        tsnClock: TsnClock {
            @display("p=500,150");
        }
        tsnSwitch: TsnSwitch {
            @display("p=500,300");
        }
        tsnDevice1: TsnDevice {
            @display("p=400,450");
        }
        tsnDevice2: TsnDevice {
            @display("p=600,450");
        }
    connections:
        tsnClock.ethg++ <--> EthernetLink <--> tsnSwitch.ethg++;
        tsnSwitch.ethg++ <--> EthernetLink <--> tsnDevice1.ethg++;
        tsnSwitch.ethg++ <--> EthernetLink <--> tsnDevice2.ethg++;
}

network TwoMasterClocksTreeGptpShowcase extends TsnNetworkBase
{
    submodules:
        tsnClock1: TsnClock {
            @display("p=500,150");
        }
        tsnClock2: TsnClock {
            @display("p=700,150");
        }
        tsnSwitch1: TsnSwitch {
            @display("p=500,300");
        }
        tsnSwitch2: TsnSwitch {
            @display("p=700,300");
        }
        tsnDevice1: TsnDevice {
            @display("p=300,450");
        }
        tsnDevice2: TsnDevice {
            @display("p=500,450");
        }
        tsnDevice3: TsnDevice {
            @display("p=700,450");
        }
        tsnDevice4: TsnDevice {
            @display("p=900,450");
        }
    connections:
        tsnClock1.ethg++ <--> EthernetLink <--> tsnSwitch1.ethg++;
        tsnClock2.ethg++ <--> EthernetLink <--> tsnSwitch2.ethg++;
        tsnSwitch1.ethg++ <--> EthernetLink <--> tsnSwitch2.ethg++;
        tsnSwitch1.ethg++ <--> EthernetLink <--> tsnDevice1.ethg++;
        tsnSwitch1.ethg++ <--> EthernetLink <--> tsnDevice2.ethg++;
        tsnSwitch2.ethg++ <--> EthernetLink <--> tsnDevice3.ethg++;
        tsnSwitch2.ethg++ <--> EthernetLink <--> tsnDevice4.ethg++;
}

network TwoMasterClocksRingGptpShowcase extends TsnNetworkBase
{
    submodules:
        scenarioManager: ScenarioManager {
            @display("p=100,500;is=s");
        }
        tsnClock1: TsnClock {
            @display("p=500,150");
        }
        tsnClock2: TsnClock {
            @display("p=900,600");
        }
        tsnSwitch1: TsnSwitch {
            @display("p=700,150");
        }
        tsnSwitch2: TsnSwitch {
            @display("p=500,300");
        }
        tsnSwitch3: TsnSwitch {
            @display("p=500,450");
        }
        tsnSwitch4: TsnSwitch {
            @display("p=700,600");
        }
        tsnSwitch5: TsnSwitch {
            @display("p=900,450");
        }
        tsnSwitch6: TsnSwitch {
            @display("p=900,300");
        }
        tsnDevice1: TsnDevice {
            @display("p=300,300");
        }
        tsnDevice2: TsnDevice {
            @display("p=300,450");
        }
        tsnDevice3: TsnDevice {
            @display("p=1100,450");
        }
        tsnDevice4: TsnDevice {
            @display("p=1100,300");
        }
    connections:
        tsnClock1.ethg++ <--> EthernetLink <--> tsnSwitch1.ethg++;
        tsnClock2.ethg++ <--> EthernetLink <--> tsnSwitch4.ethg++;
        tsnSwitch1.ethg++ <--> EthernetLink <--> tsnSwitch2.ethg++;
        tsnSwitch2.ethg++ <--> EthernetLink <--> tsnSwitch3.ethg++;
        tsnSwitch3.ethg++ <--> EthernetLink <--> tsnSwitch4.ethg++;
        tsnSwitch4.ethg++ <--> EthernetLink <--> tsnSwitch5.ethg++;
        tsnSwitch5.ethg++ <--> EthernetLink <--> tsnSwitch6.ethg++;
        tsnSwitch6.ethg++ <--> EthernetLink <--> tsnSwitch1.ethg++;
        tsnSwitch2.ethg++ <--> EthernetLink <--> tsnDevice1.ethg++;
        tsnSwitch3.ethg++ <--> EthernetLink <--> tsnDevice2.ethg++;
        tsnSwitch5.ethg++ <--> EthernetLink <--> tsnDevice3.ethg++;
        tsnSwitch6.ethg++ <--> EthernetLink <--> tsnDevice4.ethg++;
}
