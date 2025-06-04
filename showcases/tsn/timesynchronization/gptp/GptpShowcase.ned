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

network JumpingClockGptpShowcase extends TsnNetworkBase
{
    submodules:
        scenarioManager: ScenarioManager {
            @display("p=100,500;is=s");
        }
        tsnClock: TsnClock {
            @display("p=500,150");
        }
        tsnDevice: TsnDevice {
            @display("p=500,300");
        }

    connections:
        tsnClock.ethg++ <--> EthernetLink <--> tsnDevice.ethg++;
}

network MultiHopGptpShowcase extends TsnNetworkBase
{
    parameters:
        int numSwitches;
    submodules:
        scenarioManager: ScenarioManager {
            @display("p=100,800;is=s");
        }
        tsnClock: TsnClock {
            @display("p=400,150");
        }
        tsnSwitch[numSwitches]: TsnSwitch {
            @display("p=400,300,row,100");
        }
        tsnDevice: TsnDevice {
            @display("p=400,450");
        }
    connections:
        tsnClock.ethg++ <--> EthernetLink <--> tsnSwitch[0].ethg++;
        for i=0..sizeof(tsnSwitch)-2 {
            tsnSwitch[i].ethg++ <--> EthernetLink <--> tsnSwitch[i+1].ethg++;
        }
        tsnSwitch[numSwitches-1].ethg++ <--> EthernetLink <--> tsnDevice.ethg++;
}
