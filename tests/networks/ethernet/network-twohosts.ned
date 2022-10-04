package inet.tests.ethernet;

import ned.DatarateChannel;

network TwoHosts
{
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
