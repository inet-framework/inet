package inet.showcases.emulation.videostreaming2;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.visualizer.integrated.IntegratedVisualizer;

network VideoStreamingShowcase
{
    parameters:
        @display("bgb=1000,400");
        @figure[link1](type=line; points=150,300,300,300; lineOpacity=0.2);
        @figure[link2](type=line; points=700,300,850,300; lineOpacity=0.2);
        @figure[host1](type=icon; image="device/pc2"; pos=100,300; opacity=0.2);
        @figure[host2](type=icon; image="device/pc2"; pos=900,300; opacity=0.2);
        @figure[submodules];
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,200;is=s");
        }
        switch1: EtherSwitch {
            parameters:
                @display("p=300,300");
        }
        switch2: EtherSwitch {
            parameters:
                @display("p=700,300");
        }
        router: Router {
            parameters:
                @display("p=500,300");
        }
    connections allowunconnected:
        switch1.ethg++ <--> Eth100M <--> router.ethg++;
        router.ethg++ <--> Eth100M <--> switch2.ethg++;
}
