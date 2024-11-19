package inet.showcases.visualizer.osg.earth;

import inet.common.geometry.common.IGeographicCoordinateSystem;
import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.common.IntegratedVisualizer;

network EarthVisualizationShowcase
{
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        coordinateSystem: <default(firstAvailableOrEmpty("OsgGeographicCoordinateSystem", "SimpleGeographicCoordinateSystem"))> like IGeographicCoordinateSystem {
            parameters:
                @display("p=100,150");
        }
}

network EarthVisualization2 extends EarthVisualizationShowcase
{
    submodules:
        physicalEnvironment: PhysicalEnvironment {
            @display("p=100,246");
        }
}

network EarthVisualization3 extends EarthVisualizationShowcase
{
    submodules:
        physicalEnvironment: PhysicalEnvironment {
            @display("p=100,260");
        }
        host1: AdhocHost {
            @display("p=382,50");
        }
        host2: AdhocHost {
            @display("p=382,150");
        }
        host3: AdhocHost {
            @display("p=487,50");
        }
        host4: AdhocHost {
            @display("p=487,150");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=259,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=258,50");
        }
}
