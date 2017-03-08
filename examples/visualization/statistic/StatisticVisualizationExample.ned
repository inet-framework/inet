package inet.examples.visualization.statistic;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;

network StatisticVisualizationExample
{
    parameters:
        @display("bgb=1000,1000");
        @figure[description](type=text; pos=30,30; font=Arial,24; text="Demonstrates how to display a statistic for network nodes.\nSource nodes move around and ping destination node.\nRound trip time is displayed to the right of source nodes.");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=900,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=900,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=900,250");
        }
        source[3]: AdhocHost {
            parameters:
                @display("p=250,200");
        }
        destination: AdhocHost {
            parameters:
                @display("p=500,500");
        }
}
