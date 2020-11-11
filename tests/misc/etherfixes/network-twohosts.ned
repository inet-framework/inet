
import inet.node.ethernet.EthernetHost2;
import ned.DatarateChannel;


network TwoHosts
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
    connections:
        hostA.ethg <--> DatarateChannel <--> hostB.ethg;
}
