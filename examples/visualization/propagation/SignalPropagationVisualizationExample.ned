package inet.examples.visualization.propagation;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth1G;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;

network SignalPropagationVisualizationExample
{
    parameters:
        @figure[description](type=text; pos=30,30; font=Arial,24; text="Demonstrates how to display transmissions, receptions and propagating signals.");
        @display("bgb=1000,500");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,100");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,200");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,300");
        }
        source: AdhocHost {
            parameters:
                @display("p=300,200");
        }
        node: AdhocHost {
            parameters:
                @display("p=600,300");
        }
        destination: AdhocHost {
            parameters:
                @display("p=900,200");
        }
}
