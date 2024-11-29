package inet.showcases.visualizer.canvas.statistic;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211RadioMedium;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;


network StatisticVisualizationPingRttShowcase
{
    @display("bgb=400,244");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=69,40");
        }
        radioMedium: Ieee80211RadioMedium {
            @display("p=68,99");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=69,161");
        }
        source: AdhocHost {
            @display("p=178,104");
        }
        destination: AdhocHost {
            @display("p=331,104");
        }
}

network StatisticVisualizationPacketErrorRateShowcase
{
    parameters:
        @display("bgb=901.5187,376.65");
    submodules:
        visualizer: IntegratedCanvasVisualizer {
            parameters:
                @display("p=64.256,62.247997");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=64.256,180.72");
        }
        radioMedium: Ieee80211RadioMedium {
            parameters:
                @display("p=62.247997,287.14398");
        }
        destination: AdhocHost {
            parameters:
                @display("p=500,179.60625");
        }
        source: AdhocHost {
            @display("p=363.448,180.72");
        }
}
