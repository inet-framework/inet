package inet.examples.visualization.linkbreak;

import inet.visualizer.integrated.IntegratedVisualizer;

network LinkBreakVisualizationExample
{
    parameters:
        @figure[description](type=text; pos=300,10; font=Arial,20; text="TODO");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
}
