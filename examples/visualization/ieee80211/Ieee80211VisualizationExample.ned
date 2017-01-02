package inet.examples.visualization.ieee80211;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;

network Ieee80211VisualizationExample
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
        accessPointA: AccessPoint {
            parameters:
                @display("p=300,50");
        }
        accessPointB: AccessPoint {
            parameters:
                @display("p=500,50");
        }
        hostA1: WirelessHost {
            parameters:
                @display("p=300,150");
        }
        hostA2: WirelessHost {
            parameters:
                @display("p=300,250");
        }
        hostB1: WirelessHost {
            parameters:
                @display("p=500,150");
        }
        hostB2: WirelessHost {
            parameters:
                @display("p=500,250");
        }
}
