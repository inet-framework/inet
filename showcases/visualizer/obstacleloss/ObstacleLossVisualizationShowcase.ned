package inet.showcases.visualizer.obstacleloss;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;

network ObstacleLossVisualizationEnablingShowcase
{
    @display("bgb=800,400");
    submodules:
        source: AdhocHost {
            @display("p=289.44,180.90001");
        }
        destination: AdhocHost {
            @display("p=530.64,180.90001");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=88,51");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=87,116");
        }
        visualizer: IntegratedVisualizer {
            @display("p=88,181");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=88,250");
        }
}

network ObstacleLossVisualizationMultipleObstaclesShowcase
{
    @display("bgb=800,400");
    submodules:
        source: AdhocHost {
            @display("p=200,50");
        }
        destination: AdhocHost {
            @display("p=700,350");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=88,51");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=87,116");
        }
        visualizer: IntegratedVisualizer {
            @display("p=88,181");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=88,250");
        }
}
