package inet.examples.visualization.transportroute;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedVisualizer;

network TransportRouteVisualizationExample
{
    parameters:
        @display("bgb=1000,1000");
        @figure[description](type=text; pos=30,30; font=Arial,24; text="");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        standardHost: StandardHost {
            @display("p=88.352,429.71198");
        }
}
