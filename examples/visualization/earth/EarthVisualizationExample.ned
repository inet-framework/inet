package inet.examples.visualization.earth;

import inet.common.geometry.common.OsgGeographicCoordinateSystem;
import inet.visualizer.integrated.IntegratedVisualizer;

network EarthVisualizationExample
{
    parameters:
        @display("bgb=0,0");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        coordinateSystem: OsgGeographicCoordinateSystem {
            parameters:
                @display("p=100,150");
        }
}
