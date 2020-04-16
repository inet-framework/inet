package inet.showcases.visualizer.submoduleinfo;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;

network SubmoduleInformationVisualizationShowcase
{
    submodules:
        visualizer: IntegratedVisualizer {
            @display("p=100,100");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,200");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=100,300");
        }
        source: AdhocHost {
            @display("p=300,200");
        }
        destination: AdhocHost {
            @display("p=600,200");
        }
        relay: AdhocHost {
            @display("p=447,143");
        }
}
