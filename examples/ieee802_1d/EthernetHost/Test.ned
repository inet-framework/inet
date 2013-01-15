package inet.examples.ieee802_1d.EthernetHost;



import ned.DatarateChannel;
import inet.nodes.Ieee8021d.EthernetHost;
import inet.nodes.Ieee8021d.Tester;




//
// EthernetHostTester
//
network EthernetHostTester
{
    @display("bgb=689,368");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        E0: EthernetHost {
            parameters:
                @display("p=301,48");
        }
        T0: Tester {
            parameters:
                @display("p=192,118");
        }

    connections:
        E0.ethg <--> C <--> T0.Connection;
}

//
// EthernetHostNetwork
//
network EthernetHostNetwork
{
    @display("bgb=689,368");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        E0: EthernetHost {
            parameters:
                @display("p=301,48");
        }
        E1: EthernetHost {
            parameters:
                @display("p=192,118");
        }

    connections:
        E0.ethg <--> C <--> E1.ethg;
}

