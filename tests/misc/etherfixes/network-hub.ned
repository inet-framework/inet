
import inet.node.ethernet.EthernetHub;
import inet.node.ethernet.EthernetHost2;
import ned.DatarateChannel;


network HostsWithHub
{
    types:
    submodules:
        hostA: EthernetHost2 {
            parameters:
                @display("p=72,80");
        }
        hostB: EthernetHost2 {
            parameters:
                @display("p=334,80");
        }
        hostC: EthernetHost2 {
            parameters:
                @display("p=334,80");
        }
        hub: EthernetHub {
            parameters:
                @display("p=204,80");
        }
    connections:
        hub.ethg++ <--> DatarateChannel <--> hostA.ethg;
        hub.ethg++ <--> DatarateChannel <--> hostB.ethg;
        hub.ethg++ <--> DatarateChannel <--> hostC.ethg;
}
