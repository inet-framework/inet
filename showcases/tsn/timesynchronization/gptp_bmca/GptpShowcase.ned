//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.tsn.timesynchronization.gptp_bmca;

import inet.common.scenario.ScenarioManager;
import inet.networks.base.TsnNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnClock;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;

network BmcaShowcaseSimple extends TsnNetworkBase
{
    submodules:
        scenarioManager: ScenarioManager {
            @display("p=100,800;is=s");
        }
        tsnDevice1: TsnDevice {
            @display("p=500,150");
        }
        tsnSwitch: TsnSwitch {
            @display("p=500,300");
        }
        tsnDevice2: TsnDevice {
            @display("p=400,450");
        }
        tsnDevice3: TsnDevice {
            @display("p=600,450");
        }
    connections:
        tsnDevice1.ethg++ <--> EthernetLink <--> tsnSwitch.ethg++;
        tsnSwitch.ethg++ <--> EthernetLink <--> tsnDevice2.ethg++;
        tsnSwitch.ethg++ <--> EthernetLink <--> tsnDevice3.ethg++;
}

network BmcaShowcaseDiamond extends TsnNetworkBase
{
    submodules:
        scenarioManager: ScenarioManager {
            @display("p=100,800;is=s");
        }
        tsnDevice1: TsnDevice {
            @display("p=500,150");
        }
        tsnSwitchA1: TsnSwitch {
            @display("p=450,300");
        }
        tsnSwitchB1: TsnSwitch {
            @display("p=550,300");
        }
        tsnDevice2: TsnDevice {
            @display("p=500,450");
        }
    connections:
        tsnDevice1.ethg++ <--> EthernetLink <--> tsnSwitchA1.ethg++;
        tsnDevice1.ethg++ <--> EthernetLink <--> tsnSwitchB1.ethg++;
        tsnSwitchA1.ethg++ <--> EthernetLink <--> tsnDevice2.ethg++;
        tsnSwitchB1.ethg++ <--> EthernetLink <--> tsnDevice2.ethg++;
}

network BmcaShowcaseDiamondAsymmetric extends TsnNetworkBase
{
    submodules:
        scenarioManager: ScenarioManager {
            @display("p=100,800;is=s");
        }
        tsnDevice1: TsnDevice {
            @display("p=500,150");
        }
        tsnSwitchA1: TsnSwitch {
            @display("p=450,250");
        }
        tsnSwitchA2: TsnSwitch {
            @display("p=450,350");
        }
        tsnSwitchB1: TsnSwitch {
            @display("p=550,300");
        }
        tsnDevice2: TsnDevice {
            @display("p=500,450");
        }
    connections:
        tsnDevice1.ethg++ <--> EthernetLink <--> tsnSwitchA1.ethg++;
        tsnDevice1.ethg++ <--> EthernetLink <--> tsnSwitchB1.ethg++;
        tsnSwitchA1.ethg++ <--> EthernetLink <--> tsnSwitchA2.ethg++;
        tsnSwitchA2.ethg++ <--> EthernetLink <--> tsnDevice2.ethg++;
        tsnSwitchB1.ethg++ <--> EthernetLink <--> tsnDevice2.ethg++;
}
