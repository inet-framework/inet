package inet.tests.ethernet;

import inet.node.ethernet.EtherHost2;
import ned.DatarateChannel;


network TwoHosts
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
    connections:
        hostA.ethg <--> DatarateChannel <--> hostB.ethg;
}
