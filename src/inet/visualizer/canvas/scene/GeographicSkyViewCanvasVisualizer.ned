//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.scene;

import inet.visualizer.base.VisualizerBase;
import inet.visualizer.contract.IGeographicSkyViewVisualizer;

//
// Draws an azimuth/elevation "sky view" polar plot next to each observer node
// (typically a ground station using a `GeographicStationaryMobility`), showing
// where the target nodes (typically satellites) currently are in the observer's
// sky. The center is the zenith (90 deg elevation), the outer rim is the horizon
// (0 deg); North is up and East is to the right (azimuth clockwise). Concentric
// rings mark elevations, the cardinal points (N/E/S/W) are drawn, and the band
// below `minElevationAngle` is masked translucently. Each visible target is a
// colored dot (optionally named, with a fading sky trail); targets below the
// horizon are hidden.
//
// Look angles are computed from the geographic positions of the observer and the
// targets via the `IGeographicCoordinateSystem` given by `coordinateSystemModule`.
//
// @see ~GeographicMapCanvasVisualizer, ~IGeographicSkyViewVisualizer
//
simple GeographicSkyViewCanvasVisualizer extends VisualizerBase like IGeographicSkyViewVisualizer
{
    parameters:
        bool displaySkyView = default(true); // Display the sky view plots, enabled by default
        double animationSpeed = default(1); // Animation speed while targets move, value must be in the range (0, +inf)
        double zIndex = default(0); // Determines the drawing order of figures relative to other visualizers

        string coordinateSystemModule; // Module path of the geographic coordinate system used to compute look angles
        string observerFilter @mutable = default("*"); // Which mobilities are observers (get a sky plot)
        string targetFilter @mutable = default("*"); // Which mobilities are targets (drawn in each plot)

        double plotRadius = default(70); // Radius of the sky plot in pixels (elevation 0 at the rim, 90 at the center)
        double plotOffsetX = default(95); // Horizontal offset (pixels) of the plot center from the observer icon
        double plotOffsetY = default(0); // Vertical offset (pixels) of the plot center from the observer icon
        double minElevationAngle @unit(deg) = default(0deg); // Minimum usable elevation; the band between it and the horizon is masked

        string backgroundColor = default("#1a2a40"); // Sky disc fill color
        double backgroundOpacity = default(0.15); // Sky disc fill opacity
        string ringColor = default("#888888"); // Elevation ring and graticule color
        double ringLineWidth = default(1); // Elevation ring line width
        string maskColor = default("#808080"); // Fill color of the below-minimum-elevation mask
        double maskOpacity = default(0.25); // Opacity of the below-minimum-elevation mask
        string cardinalColor = default("#888888"); // Color of the N-S/W-E lines and the N/E/S/W labels
        string labelColor = default("black"); // Color of the title and elevation labels

        string targetColor = default("dark"); // Target dot color, a list of colors or a color group name (e.g. dark, light)
        double targetRadius = default(3); // Radius of a target dot in pixels
        double lowElevationOpacity = default(0.35); // Opacity of target dots between the horizon and minElevationAngle
        bool displayLabels = default(true); // Display the target name next to its dot, enabled by default
        bool displayTrails = default(true); // Display a fading sky trail of each target's recent path, enabled by default
        int trailLength = default(100); // Number of sections in a sky trail
        string trailLineStyle = default("solid"); // Sky trail line style (solid, dashed, dotted)
        double trailLineWidth = default(1); // Sky trail line width

        @class(GeographicSkyViewCanvasVisualizer);
        @display("i=block/app");
}
