package inet.tests.fingerprint.ethernet;

import inet.nodes.ethernet.EtherHost;
import ned.DatarateChannel;

network TwoHosts
{
    types:
    submodules:
        hostA: EtherHost {
            parameters:
                @display("p=72,80");
        }
        hostB: EtherHost {
            parameters:
                @display("p=334,80");
        }
    connections:
        hostA.ethg <--> DatarateChannel <--> hostB.ethg;
}

