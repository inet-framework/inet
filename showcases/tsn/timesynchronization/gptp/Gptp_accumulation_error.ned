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
        int numDevices;
    submodules:
        scenarioManager: ScenarioManager;
        tsnClock: TsnClock {
            @display("p=500,150");
        }
        tsnSwitch: TsnSwitch {
            @display("p=500,300");
        }
        tsnDevice1[numDevices]: TsnDevice {
            @display("p=400,450,col,100");
        }
        tsnDevice2[numDevices]: TsnDevice {
            @display("p=600,450,col,100");
        }
    connections:
        tsnClock.ethg++ <--> EthernetLink <--> tsnSwitch.ethg++;
        
        tsnSwitch.ethg++ <--> EthernetLink <--> tsnDevice1[0].ethg++;
        for i=0..sizeof(tsnDevice1)-2 {
            tsnDevice1[i].ethg++ <--> EthernetLink <--> tsnDevice1[i+1].ethg++;
        }
        
        tsnSwitch.ethg++ <--> EthernetLink <--> tsnDevice2[0].ethg++;
        for i=0..sizeof(tsnDevice2)-2 {
            tsnDevice2[i].ethg++ <--> EthernetLink <--> tsnDevice2[i+1].ethg++;
        }
}
