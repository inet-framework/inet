package inet.showcases.general.parsim;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;
import ned.DelayChannel;


channel Ethernet100 extends ned.DatarateChannel
{
    datarate = 100Mbps;
    delay = 100us;
//    ber = 1e-10;
}

network ParSimNetwork
{
    @display("bgb=400,400");
    submodules:
        host1: StandardHost {
            @display("p=50,200");
        }
        host2: StandardHost {
            @display("p=350,200");
        }
    connections:
//        host1.ethg++ <--> Eth100M <--> host2.ethg++;
        host1.ethg++ <--> Ethernet100 <--> host2.ethg++;
}

network ParSimNetwork2 extends ParSimNetwork
{
    submodules:

        configurator: Ipv4NetworkConfigurator {
            @display("p=35,48");
        }
}

network ParSimNetwork3 extends ParSimNetwork
{
    submodules:

        configurator1: Ipv4NetworkConfigurator {
            @display("p=50,50");
        }
        configurator2: Ipv4NetworkConfigurator {
            @display("p=350,50");
        }
}
