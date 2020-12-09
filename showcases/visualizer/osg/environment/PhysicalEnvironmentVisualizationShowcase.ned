package inet.showcases.visualizer.osg.environment;

import inet.environment.common.PhysicalEnvironment;
import inet.visualizer.common.IntegratedVisualizer;

network PhysicalEnvironmentVisualizationShowcase
{
    parameters:
        @display("bgb=0,0");
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
