package inet.showcases.visualizer.canvas.interfacetable;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.common.IntegratedVisualizer;

network InterfaceTableVisualizationEnablingVisualizationShowcase
{
    @display("bgb=400,400");
    submodules:
        hostA: StandardHost {
            @display("p=92,203");
        }
        hostB: StandardHost {
            @display("p=284,203");
        }
        visualizer: IntegratedVisualizer {
            @display("p=328,38");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=328,93");
        }
    connections:
        hostA.ethg++ <--> Eth100M <--> hostB.ethg++;
}

network InterfaceTableVisualizationAdvancedFeaturesShowcase
{
    @display("bgb=732,417");
    submodules:
        host1: StandardHost {
            @display("p=79,128");
        }
        host2: StandardHost {
            @display("p=80,218");
        }
        switch: EthernetSwitch {
            @display("p=196,181");
        }
        router: Router {
            @display("p=304,182");
        }
        accessPoint: AccessPoint {
            @display("p=439,181");
        }
        wirelessHost: WirelessHost {
            @display("p=459,80");
        }
        mobileHost: AdhocHost {
            @display("p=468,313");
        }
        configurator: Ipv4NetworkConfigurator {
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
}

