package inet.examples.visualization.interfacetable;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;

network InterfaceTableVisualizationExample
{
    parameters:
        @figure[description](type=text; pos=300,10; font=Arial,20; text="TODO");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,150");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,250");
        }
        accessPoint: AccessPoint {
            parameters:
                @display("p=300,50");
        }
        host1: WirelessHost {
            parameters:
                @display("p=300,150");
        }
        host2: WirelessHost {
            parameters:
                @display("p=300,250");
        }
}
