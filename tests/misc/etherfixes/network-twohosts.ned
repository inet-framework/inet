
import inet.node.ethernet.SimpleEthTrafGenHost;
import ned.DatarateChannel;


network TwoHosts
{
    types:
    submodules:
        hostA: SimpleEthTrafGenHost {
            parameters:
                @display("p=72,80");
        }
        hostB: SimpleEthTrafGenHost {
            parameters:
                @display("p=334,80");
        }
    connections:
        hostA.ethg <--> DatarateChannel <--> hostB.ethg;
}
