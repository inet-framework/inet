//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.showcases.visualizer.osgdemo;
import inet.common.geometry.common.OsgGeographicCoordinateSystem;
import inet.environment.common.PhysicalEnvironment;
import inet.environment.ground.OsgEarthGround;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


//
// TODO documentation
//
network osgDemoRoverShowcase
{
    parameters:
        int hosts;
        @display("bgb=5000,5000");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=120,50");
        }
        coordinateSystem: OsgGeographicCoordinateSystem {
            parameters:
                @display("p=120,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=120,250");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=120,350");
        }
        ground: OsgEarthGround {
            @display("p=120,450");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=120,550");
        }
        rover[hosts]: AdhocHost {
            @display("p=456.365,219.9225;i=misc/truck2");
        }
}

//
// TODO documentation
//
network osgDemoDroneShowcase
{
    parameters:
        int hosts;
        @display("bgb=5000,5000");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        coordinateSystem: OsgGeographicCoordinateSystem {
            parameters:
                @display("p=100,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=100,250");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,350");
        }
        drone[hosts]: AdhocHost {
            @display("p=456.365,219.9225;i=misc/drone");
        }
        ground: OsgEarthGround {
            @display("p=100.399994,477.904");
        }
}

