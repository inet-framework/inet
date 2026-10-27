//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.mobility.geographic;

import inet.common.geometry.common.Wgs84EcefGeographicCoordinateSystem;
import inet.mobility.contract.IMobility;
import inet.visualizer.canvas.common.InfoCanvasVisualizer;
import inet.visualizer.canvas.mobility.MobilityCanvasVisualizer;
import inet.visualizer.canvas.scene.GeoHorizonCanvasVisualizer;
import inet.visualizer.canvas.scene.GeoMapCanvasVisualizer;
import inet.visualizer.canvas.scene.GeoSkyViewCanvasVisualizer;
import inet.visualizer.canvas.scene.NetworkNodeCanvasVisualizer;

//
// A node that carries only a mobility submodule.
//
module GeoNode
{
    parameters:
        @networkNode;
    submodules:
        mobility: <default("inet.mobility.static.StationaryMobility")> like IMobility {
            @display("p=100,100");
        }
}

//
// A regional map of Europe, placed on the world map by its own mobility. Open the
// module to see the nodes, their trails and their footprints on the Europe map.
//
module EuropeMap
{
    parameters:
        @networkNode;
        @display("i=misc/globe;is=vs;bgb=654,622");
    submodules:
        mobility: <default("inet.mobility.static.StationaryMobility")> like IMobility {
            @display("p=820,60");
        }
        map: GeoMapCanvasVisualizer {
            @display("p=820,160");
        }
        mobilityVisualizer: MobilityCanvasVisualizer {
            @display("p=820,260");
        }
        geoHorizon: GeoHorizonCanvasVisualizer {
            @display("p=820,360");
        }
        networkNodeVisualizer: NetworkNodeCanvasVisualizer {
            @display("p=820,460");
        }
        infoVisualizer: InfoCanvasVisualizer {
            @display("p=820,560");
        }
}

network GeographicMobilityShowcase
{
    parameters:
        int numPlanes = default(0);
        int numSatellites = default(0);
        int numGroundStations = default(0);
        bool hasEuropeMap = default(false);
    submodules:
        coordinateSystem: Wgs84EcefGeographicCoordinateSystem {
            @display("p=780,40");
        }
        map: GeoMapCanvasVisualizer {
            @display("p=780,120");
        }
        planeVisualizer: MobilityCanvasVisualizer {
            @display("p=780,200");
        }
        satelliteVisualizer: MobilityCanvasVisualizer {
            @display("p=780,280");
        }
        stationVisualizer: MobilityCanvasVisualizer {
            @display("p=780,360");
        }
        geoHorizon: GeoHorizonCanvasVisualizer {
            @display("p=780,440");
        }
        skyView: GeoSkyViewCanvasVisualizer {
            @display("p=780,520");
        }
        plane[numPlanes]: GeoNode {
            @display("i=misc/airplane;is=vs");
        }
        satellite[numSatellites]: GeoNode {
            @display("i=misc/satellite2;is=vs");
        }
        groundStation[numGroundStations]: GeoNode {
            @display("i=device/receiverdish;is=s");
        }
        europe: EuropeMap if hasEuropeMap {
            @display("i=misc/globe;is=vs");
        }
}
