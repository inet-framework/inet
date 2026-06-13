//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// Base module for visualizers that project geographically located network nodes
// (satellites, ground stations, or anything carrying an `IMobility`) onto a 2D
// world map. The node scene positions are converted to geographic coordinates
// through the `IGeographicCoordinateSystem` referenced by `coordinateSystemModule`,
// and then projected to map pixels. The actual drawing is done in derived modules.
//
// @see ~GeographicMapCanvasVisualizer, ~IGeographicMapVisualizer, ~VisualizerBase
//
simple GeographicMapVisualizerBase extends VisualizerBase
{
    parameters:
        bool displayMap = default(true); // Display the map and the nodes on it, enabled by default
        double animationSpeed = default(1); // Animation speed while nodes are moving, value must be in the range (0, +inf)
        double zIndex = default(0); // Determines the drawing order of figures relative to other visualizers

        string coordinateSystemModule; // Module path of the geographic coordinate system used to convert scene positions to latitude/longitude/altitude
        string moduleFilter @mutable = default("**.mobility"); // Which mobilities are considered, matches all mobilities by default

        // map background and projection
        string projection @enum("equirectangular", "mercator") = default("equirectangular"); // Map projection: "equirectangular" (plate carree) or "mercator"
        string mapImage = default(""); // Background world map image name (looked up on the image path), empty means no raster background
        double mapWidth = default(720); // Width of the map area in pixels
        double mapHeight = default(360); // Height of the map area in pixels
        double longitudeOffset @unit(deg) = default(0deg); // Added to every longitude before projecting; use to horizontally calibrate the map when longitude 0 is not at the center of the image
        bool adjustBackgroundBox = default(true); // Set the background box (and background image) of the target module to fit the map

        // graticule (latitude/longitude grid)
        bool displayGraticule = default(true); // Display a latitude/longitude grid, enabled by default
        double graticuleLatSpacing @unit(deg) = default(30deg); // Spacing between parallels
        double graticuleLonSpacing @unit(deg) = default(30deg); // Spacing between meridians
        string graticuleLineColor = default("#EEEEEE"); // Graticule line color
        double graticuleLineWidth = default(1); // Graticule line width

        // node markers and labels
        string markerImage = default(""); // Image name for the node marker (looked up on the image path), empty means a filled circle is drawn
        double markerRadius = default(2); // Radius of the marker circle when no marker image is given
        string markerColor = default("dark"); // Marker color is a list of colors or a color group name (e.g. dark, light)
        bool displayLabels = default(true); // Display the node name next to its marker, enabled by default
        string labelColor = default("black"); // Label text color

        // orbital ground tracks
        bool displayGroundTracks = default(true); // Display the recent sub-satellite ground track, enabled by default
        int groundTrackLength = default(100); // Number of sections in the ground track
        string groundTrackLineColor = default("dark"); // Ground track line color is a list of colors or a color group name
        string groundTrackLineStyle = default("solid"); // Ground track line style (solid, dashed, dotted)
        double groundTrackLineWidth = default(1); // Ground track line width

        // coverage footprints
        bool displayFootprints = default(false); // Display the coverage footprint of each node, disabled by default
        double minElevationAngle @unit(deg) = default(0deg); // Minimum elevation angle used to compute the footprint radius
        string footprintLineColor = default("blue"); // Footprint outline color
        string footprintFillColor = default("blue"); // Footprint fill color
        double footprintOpacity = default(0.1); // Footprint opacity

        @class(GeographicMapVisualizerBase);
}
