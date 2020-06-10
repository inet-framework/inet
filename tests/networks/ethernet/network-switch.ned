package inet.tests.ethernet;

import inet.node.ethernet.EtherHost2;
import inet.node.ethernet.EtherSwitch;
import ned.TransmissionChannel;


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
        hostA.ethg <--> TransmissionChannel <--> switch.ethg++;
        hostB.ethg <--> TransmissionChannel <--> switch.ethg++;
        hostC.ethg <--> TransmissionChannel <--> switch.ethg++;
}

