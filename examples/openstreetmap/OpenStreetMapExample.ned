package inet.examples.openstreetmap;

import inet.common.geometry.common.SimpleGeographicCoordinateSystem;
import inet.environment.common.PhysicalEnvironment;
import inet.visualizer.scene.OpenStreetMapCanvasVisualizer;


network OpenStreetMapExample
{
    submodules:
        visualizer: OpenStreetMapCanvasVisualizer {
            parameters:
                @display("p=100,50");
        }
        coordinateSystem: SimpleGeographicCoordinateSystem {
            @display("p=393,148");
        }
}
