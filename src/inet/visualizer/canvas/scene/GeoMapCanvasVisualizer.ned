//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.scene;

import inet.visualizer.base.VisualizerBase;
import inet.visualizer.contract.IGeoMapVisualizer;

//
// Draws a 2D world map on the OMNeT++ canvas: a background map image stretched to
// the map area and optionally overlaid with a latitude/longitude graticule, plus an
// optional absolute UTC wall-clock in the lower-left corner. It also sets up the
// shared canvas projection so that other canvas visualizers drawing scene
// coordinates (node markers, movement trails, `GeoHorizonCanvasVisualizer` footprints,
// ...) line up with this map.
//
// The map image and the graticule are drawn together as one group at this
// visualizer's `zIndex`, with the graticule on top of the image. No per-node markers
// are drawn here; node visualization is left to the other visualizers.
//
// @see ~IGeoMapVisualizer, ~GeoHorizonCanvasVisualizer, ~MobilityCanvasVisualizer
//
simple GeoMapCanvasVisualizer extends VisualizerBase like IGeoMapVisualizer
{
    parameters:
        bool displayMap = default(true); // Display the map, enabled by default
        double zIndex = default(-16); // Determines the drawing order of figures relative to other visualizers

        // map background and projection (equirectangular / plate-carree)
        string mapImage = default(""); // Background world map image name (looked up on the image path), empty means no raster background
        double mapWidth = default(720); // Width of the map area in pixels
        double mapHeight = default(360); // Height of the map area in pixels
        double minLatitude @unit(deg) = default(-90deg); // Southern edge of the visible map window
        double maxLatitude @unit(deg) = default(90deg); // Northern edge of the visible map window
        double minLongitude @unit(deg) = default(-180deg); // Western edge of the visible map window
        double maxLongitude @unit(deg) = default(180deg); // Eastern edge of the visible map window; if less than or equal to minLongitude the window wraps across the antimeridian
        bool adjustBackgroundBox = default(true); // Set the background box (and background image) of the target module to fit the map
        bool clipFigures = default(true); // Clip other canvas visualizers' figures (node icons, mobility markers/trails, links, routes, ...) to the map area, so a limited/regional map window does not show figures projected outside it

        // graticule (latitude/longitude grid)
        bool displayGraticule = default(true); // Display a latitude/longitude grid, enabled by default
        double graticuleLatSpacing @unit(deg) = default(30deg); // Spacing between parallels
        double graticuleLonSpacing @unit(deg) = default(30deg); // Spacing between meridians
        string graticuleLineColor = default("#EEEEEE"); // Graticule line color
        double graticuleLineWidth = default(1); // Graticule line width

        // absolute wall-clock (shown in the lower-left corner)
        string epoch = default(""); // Absolute UTC time (ISO-8601, e.g. "2026-06-02T00:00:00Z") mapped to simulation time 0; drives the absolute date/time shown in the lower-left corner; empty means no clock
        string clockColor = default("black"); // Color of the wall-clock text
        @class(GeoMapCanvasVisualizer);
}
