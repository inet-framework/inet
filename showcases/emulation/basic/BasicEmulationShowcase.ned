package inet.showcases.emulation.basic;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;

//
// Both host1 and host2 are simulated but may be partially external.
//
network NoExtHostsShowcase
{
    parameters:
        @display("bgb=600,300");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        host1: StandardHost {
            parameters:
                @display("p=300,150");
        }
        host2: StandardHost {
            parameters:
                @display("p=500,150");
        }
    connections allowunconnected:
        host1.ethg++ <--> Eth100M <--> host2.ethg++;
}

//
// Host1 is simulated and Host2 is external.
//
network ExtHost2Showcase
{
    parameters:
        @display("bgb=600,300");
        @figure[link](type=line; points=320,150,480,150; lineOpacity=0.2);
        @figure[host2](type=icon; image="device/pc2"; pos=500,150; opacity=0.2);
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        host1: StandardHost {
            parameters:
                @display("p=300,150");
        }
    connections allowunconnected:
}

//
// Host2 is external
//
network ExtHost1Showcase
{
    parameters:
        @display("bgb=600,300");
        @figure[link](type=line; points=320,150,480,150; lineOpacity=0.2);
        @figure[host1](type=icon; image="device/pc2"; pos=300,150; opacity=0.2);
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        host2: StandardHost {
            parameters:
                @display("p=500,150");
        }
    connections allowunconnected:
}
