



package inet.samples.visualizer.interfacetablevisualizer;

import inet.node.aodv.AODVRouter;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;
import inet.visualizer.linklayer.InterfaceTableVisualizer;


network Simple
{
    @display("bgb=400,400");
    submodules:
        hostA: StandardHost {
            @display("p=92,203");
        }
        hostB: StandardHost {
            @display("p=233,203");
        }
        visualizer: IntegratedVisualizer {
            @display("p=328,38");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=328,93");
        }
    connections:
        hostA.ethg++ <--> Eth100M <--> hostB.ethg++;
}

network SimpleV2
{
    @display("bgb=400,400");
    submodules:
        hostA: StandardHost {
            @display("p=166,124");
        }
        hostB: StandardHost {
            @display("p=166,299");
        }
        visualizer: IntegratedVisualizer {
            @display("p=328,38");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=328,93");
        }
    connections:
        hostA.ethg++ <--> Eth100M <--> hostB.ethg++;
}

network Complex
{
    @display("bgb=732,417");
    submodules:
        host1: StandardHost {
            @display("p=58,103");
        }
        host2: StandardHost {
            @display("p=59,193");
        }
        switch: EtherSwitch {
            @display("p=175,156");
        }
        router: Router {
            @display("p=283,157");
        }
        accessPoint: AccessPoint {
            @display("p=418,156");
        }
        wirelessHost: WirelessHost {
            @display("p=438,55");
        }
        mobileHost: AODVRouter {
            @display("p=447,288");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=631,50");
        }
        visualizer: IntegratedVisualizer {
            @display("p=631,122");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=630,182;b=11,5");
        }
    connections:
        host1.ethg++ <--> Eth100M <--> switch.ethg++;
        host2.ethg++ <--> Eth100M <--> switch.ethg++;
        switch.ethg++ <--> Eth100M <--> router.ethg++;
        router.ethg++ <--> Eth100M <--> accessPoint.ethg++;
        accessPoint.ethg++ <--> Eth100M <--> wirelessHost.ethg++;
}

