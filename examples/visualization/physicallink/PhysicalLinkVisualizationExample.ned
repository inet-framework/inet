package inet.examples.visualization.physicallink;

import inet.visualizer.integrated.IntegratedVisualizer;

network TransportConnectionVisualizationExample
{
    parameters:
        @figure[description](type=text; pos=300,10; font=Arial,20; text="TODO");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
}
