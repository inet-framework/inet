
import inet.linklayer.ethernet.EtherHub;
import inet.node.ethernet.EtherHost2;
import ned.DatarateChannel;


network HostsWithHub
{
    types:
    submodules:
        hostA: EtherHost2 {
            parameters:
                @display("p=72,80");
        }
        hostB: EtherHost2 {
            parameters:
                @display("p=334,80");
        }
        hostC: EtherHost2 {
            parameters:
                @display("p=334,80");
        }
        hub: EtherHub {
            parameters:
                @display("p=204,80");
        }
    connections:
        hub.ethg++ <--> DatarateChannel <--> hostA.ethg;
        hub.ethg++ <--> DatarateChannel <--> hostB.ethg;
        hub.ethg++ <--> DatarateChannel <--> hostC.ethg;
}
