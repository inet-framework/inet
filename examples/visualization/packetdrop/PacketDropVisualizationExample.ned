package inet.examples.visualization.packetdrop;

import inet.visualizer.integrated.IntegratedVisualizer;

network PacketDropVisualizationExample
{
    parameters:
        @figure[description](type=text; pos=300,10; font=Arial,20; text="TODO");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
}
