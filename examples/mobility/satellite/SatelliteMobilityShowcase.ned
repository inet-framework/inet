//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.mobility.satellite;

import inet.common.geometry.common.GeocentricCoordinateSystem;
import inet.mobility.contract.IMobility;
import inet.mobility.satellite.SatelliteController;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;

//
// A minimal node carrying only a mobility submodule, used to demonstrate the
// satellite and geographic mobility models without a full protocol stack.
//
module MobileNode
{
    parameters:
        @display("i=misc/sat");
        @networkNode;
    submodules:
        mobility: <default("inet.mobility.static.StationaryMobility")> like IMobility {
            @display("p=100,100");
        }
}

//
// Demonstrates SGP4-based satellite mobility and geographic ground placement in a
// geocentric (WGS84/ECEF) scene. The controller reads a TLE file and inserts a
// satellite node per orbit at simulation start; the ground station is placed by
// latitude/longitude using the same coordinate system.
//
network SatelliteMobilityShowcase
{
    submodules:
        // utility modules placed to the right of the map; the row below the map
        // (y=400) is left free for the dynamically created satellite[*] icons
        coordinateSystem: GeocentricCoordinateSystem {
            @display("p=780,40");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=780,120");
        }
        controller: SatelliteController {
            @display("p=780,200");
        }
        groundStation: MobileNode {
            @display("p=0,0");
        }
}
