package inet.tests.ethernet;

import inet.node.ethernet.EthernetSwitch;
import ned.DatarateChannel;

network HostsWithSwitch
{
    @display("bgl=2");
    submodules:
        hostA: EthernetHost2 {
            parameters:
                @display("p=113,43");
        }
        hostB: EthernetHost2 {
            parameters:
                @display("p=106,136");
        }
        hostC: EthernetHost2 {
            parameters:
                @display("p=397,68");
        }
        switch: EthernetSwitch {
            parameters:
                @display("p=260,79");
        }
    connections:
        hostA.ethg <--> DatarateChannel <--> switch.ethg++;
        hostB.ethg <--> DatarateChannel <--> switch.ethg++;
        hostC.ethg <--> DatarateChannel <--> switch.ethg++;
}
