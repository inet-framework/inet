package inet.examples.openstreetmap;

import inet.common.geometry.common.SimpleGeographicCoordinateSystem;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;

network OpenStreetMapExample
{
    submodules:
        visualizer: IntegratedCanvasVisualizer {
            parameters:
                @display("p=100,100");
        }
        coordinateSystem: SimpleGeographicCoordinateSystem {
            parameters:
                @display("p=100,200");
        }
}
