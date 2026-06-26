//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.mobility;

import inet.common.geometry.common.Wgs84EcefGeographicCoordinateSystem;
import inet.common.geometry.common.Wgs84AnchoredGeographicCoordinateSystem;
import inet.mobility.contract.IMobility;
import inet.visualizer.canvas.mobility.MobilityCanvasVisualizer;
import inet.visualizer.canvas.scene.GeoHorizonCanvasVisualizer;
import inet.visualizer.canvas.scene.GeoMapCanvasVisualizer;
import inet.visualizer.canvas.scene.GeoSkyViewCanvasVisualizer;
import inet.visualizer.canvas.scene.NetworkNodeCanvasVisualizer;
import inet.visualizer.canvas.scene.OpenStreetMapSceneCanvasVisualizer;
import inet.visualizer.canvas.common.InfoCanvasVisualizer;

//
// A minimal node carrying only a mobility submodule, used to demonstrate the
// satellite and geographic mobility models without a full protocol stack.
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
// The most detailed level of the drill-down: a local street-level scene over the
// Óbuda district of Budapest, shown on its own canvas (open the module to see it).
// It is placed on the Europe map by its `mobility`, and carries an OpenStreetMap
// backdrop drawn through the local (anchored) `coordinateSystem`.
//
module Budapest extends GeoNode
{
    parameters:
        @display("i=misc/town;is=vs;bgb=800,800"); // shown as a town icon on the Europe map; open it to see the OpenStreetMap view
    submodules:
        coordinateSystem: Wgs84AnchoredGeographicCoordinateSystem { // used only to project the OSM backdrop onto this canvas
            @display("p=60,60");
        }
        // a single-purpose map visualizer (no node/scene visualizers, which would track this
        // @networkNode and clutter/break its own canvas); it draws the OSM backdrop on this canvas
        map: OpenStreetMapSceneCanvasVisualizer { // OpenStreetMap scene, drawn on this module's canvas
            @display("p=60,160");
        }
}

//
// The middle level of the drill-down: a regional zoom over Europe, shown on its own
// canvas (open the module to see it). It is placed on the world map by its `mobility`,
// and carries a geographic map visualizer that draws the Europe map on this module's
// canvas. The `budapest` module inside it is in turn placed on the Europe map, so the
// user can progressively open more and more detailed maps (world -> Europe -> Óbuda).
//
module Europe extends GeoNode
{
    parameters:
        @display("i=misc/globe;is=vs;bgb=654,622"); // shown as a globe icon on the world map; open it to see the Europe map
    submodules:
        // a single-purpose map visualizer (no node/scene visualizers, which would track this
        // @networkNode and crash at cleanup); it draws the Europe map on this module's canvas and
        // sets up the equirectangular projection so the budapest child lands at its map position
        map: GeoMapCanvasVisualizer { // Europe map, drawn on this module's canvas
            @display("p=60,60");
        }
        // draws the satellites (position markers, labels, ground-track trails) on THIS
        // module's canvas; its visualizationSubjectModule is pointed at the network in the
        // ini so it discovers the network-level satellite[*] nodes, while still drawing on
        // the Europe canvas (visualizationTargetModule defaults to this parent module)
        satVisualizer: MobilityCanvasVisualizer { // satellites shown on the Europe map
            @display("p=60,260");
        }
        // draws the ground stations (position markers, labels) on THIS module's canvas
        gsVisualizer: MobilityCanvasVisualizer { // ground stations shown on the Europe map
            @display("p=60,260");
        }
        // draws the satellite visibility footprints on this module's canvas, aligned to the
        // Europe map ('map' sibling) and clipped to its window; subject is the network (ini)
        geoHorizon: GeoHorizonCanvasVisualizer { // satellite footprints on the Europe map
            @display("p=60,360");
        }
        // draws the managed node icons (sat/plane/gs) on the Europe map; watches the network nodes
        // (visualizationSubjectModule in the ini) and is filtered to exclude the europe @networkNode
        // itself (tracking its own ancestor would crash at cleanup)
        networkNodeVisualizer: NetworkNodeCanvasVisualizer { // node icons on the Europe map
            @display("p=60,460");
        }
        // labels each node icon with its display-name (TEME, ISS, Lima, JFK - LHR, ...); on the
        // world canvas Qtenv draws these names itself, but on this sub-canvas they must be drawn
        // explicitly. It annotates the networkNodeVisualizer's node figures (sibling, the default).
        infoVisualizer: InfoCanvasVisualizer { // node name labels on the Europe map
            @display("p=60,560");
        }
        budapest: Budapest { // placed on the Europe map by its own mobility
            @display("i=misc/town;is=vs");
        }
}

//
// The satellite showcase: the geocentric (WGS84/ECEF) coordinate system shared by all
// nodes, the visualizers that draw the world map, the satellites, their footprints and
// the ground-station sky views, the satellite nodes, two ground stations, and the
// `europe` drill-down module placed on the world map. Each satellite is configured
// individually (its TLE file, the selected satellite, and the UTC epoch) in the ini.
//
// The visualizers are instantiated directly (rather than via an IntegratedCanvasVisualizer)
// so the example only pulls in the ones it needs; each draws on the network canvas
// (visualizationTargetModule defaults to the parent) and watches the network nodes.
//
network SatelliteNetwork
{
    submodules:
        // utility modules placed to the right of the map
        coordinateSystem: Wgs84EcefGeographicCoordinateSystem {
            @display("p=780,40");
        }
        // the world map drawn on the network canvas; also sets up the shared equirectangular
        // projection that the node icons, satellite trails and footprints all use
        map: GeoMapCanvasVisualizer {
            @display("p=780,120");
        }
        // the satellites: position markers, name labels, ground-track trails, orientation
        // cones and velocity arrows (restricted to satellite[*] in the ini)
        satVisualizer: MobilityCanvasVisualizer {
            @display("p=780,200");
        }
        // the satellite visibility footprints, aligned to the world map ('map' sibling)
        geoHorizon: GeoHorizonCanvasVisualizer {
            @display("p=780,280");
        }
        // the az/el sky-view polar plots next to the ground stations
        skyView: GeoSkyViewCanvasVisualizer {
            @display("p=780,360");
        }
        // marks the ground-station locations on the world map (their icons sit in a row below the
        // map, so this draws a position marker at each station's projected geographic position)
        gsVisualizer: MobilityCanvasVisualizer {
            @display("p=780,440");
        }
        // the aircraft: their location and ground-track movement trail on the world map
        planeVisualizer: MobilityCanvasVisualizer {
            @display("p=780,520");
        }
        // one node per satellite in satellites.tle (index 0 = Vanguard 1, index 1 = ISS);
        // laid out in the row below the map (y=400)
        sat[2]: GeoNode {
            @display("i=misc/satellite2;is=vs");
        }
        // ground stations placed at their geographic position on the map by their mobility
        // (updateDisplayString), labeled by their display-name (configured per index in the ini).
        // No 'row' display-string arrangement here: it would add a per-index offset on top of the
        // mobility-set position (only x,y are overwritten), pushing gs[1], gs[2]... off their spot.
        gs[2]: GeoNode {
            @display("i=device/receiverdish;is=s");
        }
        // aircraft flying great-circle routes between major cities (GnssTrackMobility); shown
        // on the world map with their location and movement trail
        plane[4]: GeoNode {
            @display("i=misc/airplane;is=vs");
        }
        // regional drill-down: open it to see the Europe map, and the Óbuda OSM view inside it
        europe: Europe {
            @display("i=misc/globe;is=vs");
        }
}
