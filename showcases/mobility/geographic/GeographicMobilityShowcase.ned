//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.mobility.geographic;

import inet.common.geometry.common.Wgs84AnchoredGeographicCoordinateSystem;
import inet.common.geometry.common.Wgs84EcefGeographicCoordinateSystem;
import inet.mobility.contract.IMobility;
import inet.visualizer.canvas.common.InfoCanvasVisualizer;
import inet.visualizer.canvas.mobility.MobilityCanvasVisualizer;
import inet.visualizer.canvas.scene.GeoHorizonCanvasVisualizer;
import inet.visualizer.canvas.scene.GeoMapCanvasVisualizer;
import inet.visualizer.canvas.scene.GeoSkyViewCanvasVisualizer;
import inet.visualizer.canvas.scene.NetworkNodeCanvasVisualizer;
import inet.visualizer.canvas.scene.OpenStreetMapSceneCanvasVisualizer;

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
// A street-level OpenStreetMap view of the Óbuda district of Budapest, placed on the
// Europe map by its own mobility. Open the module to see the map.
//
module BudapestMap extends GeoNode
{
    parameters:
        @display("i=misc/town;is=vs;bgb=800,800");
    submodules:
        coordinateSystem: Wgs84AnchoredGeographicCoordinateSystem {
            @display("p=60,60");
        }
        map: OpenStreetMapSceneCanvasVisualizer {
            @display("p=60,160");
        }
}

//
// A regional map of Europe, placed on the world map by its own mobility. Open the
// module to see the nodes, their trails and their footprints on the Europe map, and
// the Budapest module that opens the street-level view.
//
module EuropeMap extends GeoNode
{
    parameters:
        @display("i=misc/globe;is=vs;bgb=654,622");
    submodules:
        map: GeoMapCanvasVisualizer {
            @display("p=60,60");
        }
        mobilityVisualizer: MobilityCanvasVisualizer {
            @display("p=60,160");
        }
        geoHorizon: GeoHorizonCanvasVisualizer {
            @display("p=60,260");
        }
        networkNodeVisualizer: NetworkNodeCanvasVisualizer {
            @display("p=60,360");
        }
        infoVisualizer: InfoCanvasVisualizer {
            @display("p=60,460");
        }
        budapest: BudapestMap {
            @display("i=misc/town;is=vs");
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
