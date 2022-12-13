
import inet.node.ethernet.EthernetHub;
import inet.node.ethernet.SimpleEthTrafGenHost;
import ned.DatarateChannel;


network HostsWithHub
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
        hostC: SimpleEthTrafGenHost {
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
