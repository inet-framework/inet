package inet.examples.visualization.mobility;

import inet.node.inet.WirelessHost;
import inet.visualizer.integrated.IntegratedVisualizer;

network MobilityVisualizationExample
{
    parameters:
        @display("bgb=1000,1000");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        node[20]: WirelessHost {
            parameters:
                @display("r=,,#707070");
        }
}
