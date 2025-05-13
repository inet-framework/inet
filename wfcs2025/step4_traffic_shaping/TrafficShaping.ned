package step4_traffic_shaping;

import Wfcs2025NetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;

network TrafficShapingShowcase extends Wfcs2025NetworkBase
{
    submodules:
        client1: TsnDevice {
            @display("p=500,100");
        }
        client2: TsnDevice {
            @display("p=500,300");
        }
        switch: TsnSwitch {
            @display("p=800,200");
        }
        server: TsnDevice {
            @display("p=1100,200");
        }
    connections:
        client1.ethg++ <--> EthernetLink <--> switch.ethg++;
        client2.ethg++ <--> EthernetLink <--> switch.ethg++;
        switch.ethg++ <--> EthernetLink <--> server.ethg++;
}

