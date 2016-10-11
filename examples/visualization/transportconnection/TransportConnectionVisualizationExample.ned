package inet.examples.visualization.transportconnection;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;

network TransportConnectionVisualizationExample
{
    parameters:
        @figure[description](type=text; pos=300,10; font=Arial,20; text="TODO");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,250");
        }
        host[10]: AdhocHost {
            parameters:
                @display("r=,,#707070");
        }
}
