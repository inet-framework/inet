package step1_cutthrough_switching;

import Wfcs2025NetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;

network CutthroughSwitchingShowcase extends Wfcs2025NetworkBase
{
    submodules:
        source: TsnDevice {
            @display("p=500,100");
        }
        switch1: TsnSwitch {
            @display("p=800,100");
        }
        switch2: TsnSwitch {
            @display("p=1100,100");
        }
        destination: TsnDevice {
            @display("p=1400,100");
        }
    connections:
        source.ethg++ <--> EthernetLink <--> switch1.ethg++;
        switch1.ethg++ <--> EthernetLink <--> switch2.ethg++;
        switch2.ethg++ <--> EthernetLink <--> destination.ethg++;
}

