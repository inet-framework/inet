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

network OneMasterClockGptpAccumulationError extends TsnNetworkBase
{
    parameters:
        int numSwitch;
    submodules:
        scenarioManager: ScenarioManager;
        tsnClock: TsnClock {
            @display("p=500,150");
        }
        tsnSwitch[numSwitch]: TsnSwitch {
            @display("p=400,450,col,100");
        }
        tsnDevice: TsnDevice {
            @display("p=600,450,col,100");
        }
    connections:
        tsnClock.ethg++ <--> EthernetLink <--> tsnSwitch[0].ethg++;
        for i=0..sizeof(tsnSwitch)-2 {
            tsnSwitch[i].ethg++ <--> EthernetLink <--> tsnSwitch[i+1].ethg++;
        }
        tsnSwitch[numSwitch-1].ethg++ <--> EthernetLink <--> tsnDevice.ethg++;
}
