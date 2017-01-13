package inet.examples.visualization.advanced;

import inet.environment.common.PhysicalEnvironment;
import inet.visualizer.environment.PhysicalEnvironmentCanvasVisualizer;
import inet.visualizer.environment.PhysicalEnvironmentOsgVisualizer;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;
import inet.visualizer.integrated.IntegratedVisualizer;

network SimpleVisualizationExample
{
    parameters:
        @display("bgb=0,0");
    submodules:
        physicalEnvironment: PhysicalEnvironment {
            parameters:
                @display("p=100,50");
        }
        canvasVisualizer: PhysicalEnvironmentCanvasVisualizer {
            parameters:
                @display("p=100,150");
        }
        osgVisualizer: PhysicalEnvironmentOsgVisualizer {
            parameters:
                @display("p=100,250");
        }
}

network DisableVisualizationsExample
{
    parameters:
        @display("bgb=0,0");
    submodules:
        physicalEnvironment: PhysicalEnvironment {
            parameters:
                @display("p=100,50");
        }
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,150");
        }
}

network MultipleVisualizationsExample
{
    parameters:
        @display("bgb=0,0");
    submodules:
        physicalEnvironment: PhysicalEnvironment {
            parameters:
                @display("p=100,50");
        }
        topVisualizer: IntegratedCanvasVisualizer {
            parameters:
                @display("p=100,150");
        }
        isometricVisualizer: IntegratedCanvasVisualizer {
            parameters:
                @display("p=100,250");
        }
}
