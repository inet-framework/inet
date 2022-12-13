
import inet.node.ethernet.SimpleEthTrafGenHost;
import inet.node.ethernet.EthernetSwitch;
import ned.DatarateChannel;


network HostsWithSwitch
{
    @display("bgl=2");
    types:
    submodules:
        hostA: SimpleEthTrafGenHost {
            parameters:
                @display("p=113,43");
        }
        hostB: SimpleEthTrafGenHost {
            parameters:
                @display("p=106,136");
        }
        hostC: SimpleEthTrafGenHost {
            parameters:
                @display("p=397,68");
        }
        switch: EthernetSwitch {
            parameters:
                @display("p=260,79");
        }
    connections:
        hostA.ethg <--> DatarateChannel <--> switch.ethg++;
        hostB.ethg <--> DatarateChannel <--> switch.ethg++;
        hostC.ethg <--> DatarateChannel <--> switch.ethg++;
}
