package step6_time_synchronization;

import Wfcs2025NetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnClock;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;

network TimeSynchronizationShowcase extends Wfcs2025NetworkBase
{
    submodules:
        masterClock: TsnClock {
            @display("p=700,100");
        }
        switch: TsnSwitch {
            @display("p=700,300");
        }
        device1: TsnDevice {
            @display("p=500,500");
        }
        device2: TsnDevice {
            @display("p=900,500");
        }
    connections:
        masterClock.ethg++ <--> EthernetLink <--> switch.ethg++;
        switch.ethg++ <--> EthernetLink <--> device1.ethg++;
        switch.ethg++ <--> EthernetLink <--> device2.ethg++;
}
