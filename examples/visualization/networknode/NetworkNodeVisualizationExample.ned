package inet.examples.visualization.networknode;

import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedVisualizer;

network NetworkNodeVisualizationExample
{
    parameters:
        @display("bgb=1000,1000");
        @figure[description](type=text; pos=30,30; font=Arial,24; text="Demonstrates how to display external 3D models for network nodes.");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,200");
        }
        node[10]: StandardHost {
            parameters:
                @display("r=,,#707070");
        }
}
