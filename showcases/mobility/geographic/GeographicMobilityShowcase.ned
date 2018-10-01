package inet.showcases.mobility.geographic;

import inet.common.geometry.common.OsgGeographicCoordinateSystem;
import inet.node.inet.AdhocHost;
import inet.visualizer.integrated.IntegratedVisualizer;

network GeographicMobilityShowcase
{
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        coordinateSystem: OsgGeographicCoordinateSystem {
            parameters:
                @display("p=100,150");
        }
        host: AdhocHost {
            parameters:
                @display("p=300,150");
        }
}
