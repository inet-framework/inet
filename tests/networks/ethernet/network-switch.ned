package inet.tests.ethernet;

import inet.node.ethernet.EtherHost2;
import inet.node.ethernet.EtherSwitch;
import ned.DatarateChannel;


network HostsWithSwitch
{
    @display("bgl=2");
    types:
    submodules:
        hostA: EtherHost2 {
            parameters:
                @display("p=113,43");
        }
        hostB: EtherHost2 {
            parameters:
                @display("p=106,136");
        }
        hostC: EtherHost2 {
            parameters:
                @display("p=397,68");
        }
        switch: EtherSwitch {
            parameters:
                @display("p=260,79");
        }
    connections:
        hostA.ethg <--> DatarateChannel <--> switch.ethg++;
        hostB.ethg <--> DatarateChannel <--> switch.ethg++;
        hostC.ethg <--> DatarateChannel <--> switch.ethg++;
}
