//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.scene;

import inet.visualizer.base.VisualizerBase;
import inet.visualizer.contract.IGeoHorizonVisualizer;

//
// Draws the satellite visibility footprint of the tracked nodes on top of an
// equirectangular map (e.g. a `GeoMapCanvasVisualizer`): the area on the
// Earth from which a node is seen above the `elevationMask`. The footprint is
// rebuilt every refresh from each node's current position.
//
// Node scene positions are interpreted in the geocentric ECEF frame, so the
// referenced map visualizer's `coordinateSystemModule` should point to a
// `Wgs84EcefGeographicCoordinateSystem` (as used by `SatelliteMobility`). The map
// window and the coordinate system are taken from the referenced `mapVisualizer`
// so that the footprint always lines up with the map it is drawn on.
//
// @see ~GeoMapCanvasVisualizer, ~MobilityCanvasVisualizer
//
simple GeoHorizonCanvasVisualizer extends VisualizerBase like IGeoHorizonVisualizer
{
    parameters:
        double zIndex = default(-15); // Drawing order; above the background map by default
        double animationSpeed = default(1); // Animation speed while nodes are moving, value must be in the range (0, +inf)
        string coordinateSystemModule; // Module path of the geographic (ECEF) coordinate system used to place the footprints (e.g. the network's Wgs84EcefGeographicCoordinateSystem)
        string mapVisualizer = default("^.geoMapVisualizer"); // Module path of the map visualizer (a GeoMapCanvasVisualizer) this overlay is aligned to; its map window is used. Default matches the IntegratedCanvasVisualizer sibling; set explicitly for other layouts.
        string moduleFilter @mutable = default("**.mobility"); // Which mobilities are considered, matches all mobilities by default

        // visibility footprint
        bool displayVisibility = default(true); // Display the visibility footprint of each node, enabled by default
        double elevationMask @unit(deg) = default(0deg); // Minimum elevation angle used to compute the visibility footprint radius
        int footprintSampleCount = default(72); // Number of sample points around the footprint circle
        string visibilityFillColor = default("blue"); // Visibility footprint fill color
        string visibilityLineColor = default("blue"); // Visibility footprint outline color
        double visibilityOpacity = default(0.1); // Visibility footprint opacity

        @class(GeoHorizonCanvasVisualizer);
        @display("i=block/app");
}
