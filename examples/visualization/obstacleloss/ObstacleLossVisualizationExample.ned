package inet.examples.visualization.obstacleloss;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;

network ObstacleLossVisualizationExample
{
    parameters:
        @display("bgb=1000,1000");
        @figure[description](type=text; pos=30,30; font=Arial,24; text="Demonstrates how to display obstacle loss of radio signals.");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,100");
        }
        physicalEnvironment: PhysicalEnvironment {
            parameters:
                @display("p=100,200");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,300");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,400");
        }
        source: AdhocHost {
            parameters:
                @display("p=250,150");
        }
        destination: AdhocHost {
            parameters:
                @display("p=400,150");
        }
}
