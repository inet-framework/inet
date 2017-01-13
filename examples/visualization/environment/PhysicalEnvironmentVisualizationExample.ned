package inet.examples.visualization.environment;

import inet.environment.common.PhysicalEnvironment;
import inet.visualizer.integrated.IntegratedVisualizer;

network PhysicalEnvironmentVisualizationExample
{
    parameters:
        @display("bgb=0,0");
        @figure[description](type=text; pos=30,30; font=Arial,24; text="Demonstrates how to display physical objects present in the physical environment.");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        physicalEnvironment: PhysicalEnvironment {
            parameters:
                @display("p=100,150");
        }
}
