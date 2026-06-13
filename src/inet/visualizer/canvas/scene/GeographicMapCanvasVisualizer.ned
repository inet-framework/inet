//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.scene;

import inet.visualizer.base.GeographicMapVisualizerBase;
import inet.visualizer.contract.IGeographicMapVisualizer;

//
// Projects geographically located network nodes (satellites, ground stations, or
// anything carrying an `IMobility`) onto a 2D world map drawn on the OMNeT++
// canvas. The map background is the target module's background image, sized to
// the map area and optionally overlaid with a latitude/longitude graticule. Each
// tracked node is drawn at its sub-satellite point as a marker (image or circle),
// optionally with a name label, a trailing orbital ground track, and a coverage
// footprint.
//
// Node scene positions are converted to latitude/longitude/altitude through the
// `IGeographicCoordinateSystem` given by `coordinateSystemModule` (e.g. a
// `GeocentricCoordinateSystem`), so the visualizer works for both satellites and
// ground stations without any satellite-specific dependency.
//
// @see ~GeographicMapVisualizerBase, ~MobilityCanvasVisualizer
//
simple GeographicMapCanvasVisualizer extends GeographicMapVisualizerBase like IGeographicMapVisualizer
{
    parameters:
        // absolute wall-clock (shown in the lower-left corner)
        string satelliteController = default(""); // Module path of the SatelliteController whose 'epoch' parameter (ISO-8601 UTC at simulation time 0) drives the absolute date/time shown in the lower-left corner; empty means no clock
        string clockColor = default("black"); // Color of the wall-clock text
        @class(GeographicMapCanvasVisualizer);
}
