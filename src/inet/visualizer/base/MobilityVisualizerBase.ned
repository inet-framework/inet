//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for mobility visualizer simple modules. It visualizes
// the movement, position, orientation, and velocity of network nodes in the
// simulation.
//
// The visualizer offers multiple visualization options:
// - Current positions as colored circles
// - Current orientations as pie slices
// - Current velocities as arrows
// - Movement trails showing the recent path of nodes
//
// By default, only basic mobility visualization is enabled. The additional
// visualizations (positions, orientations, velocities, and trails) can be
// enabled individually through their corresponding display parameters.
//
// This module subscribes to the `mobilityStateChanged` signal in the module
// determined by the `visualizationSubjectModule` parameter. The actual rendering
// of the visualizations is implemented in derived modules for specific user
// interfaces (canvas or OSG).
//
// The mobility visualizer is particularly useful for understanding node movement
// patterns, analyzing the impact of mobility on network performance, and
// debugging mobility models.
//
// @see ~MobilityCanvasVisualizer, ~MobilityOsgVisualizer, ~IMobilityVisualizer, ~VisualizerBase
//
simple MobilityVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // The network node visualizer module, refers to a sibling submodule by default

        bool displayMobility = default(true); // Display mobility, enabled by default
        double animationSpeed = default(0); // Animation speed while objects are moving around, value must be in the range (0, +inf)

        string moduleFilter @mutable = default("**.mobility"); // Which mobilities are considered, matches all mobilities by default

        bool displayPositions = default(false); // Display a circle indicating the current position, disabled by default
        double positionCircleRadius = default(4); // Radius of the circle
        double positionCircleLineWidth = default(1); // Line width of the circle
        string positionCircleLineColor = default("dark"); // Position circle line color is a list of colors or a color group name (e.g. dark, light), a set of dark colors by default
        string positionCircleFillColor = default("dark"); // Position circle fill color is a list of colors or a color group name (e.g. dark, light), a set of dark colors by default

        bool displayOrientations = default(false); // Display a pie indicating the current orientation, disabled by default
        double orientationPieRadius = default(32); // Radius of the pie
        double orientationPieSize = default(0.2); // Relative size of the pie compared to the full circle
        double orientationPieOpacity = default(0.25); // Opacity of the pie
        string orientationLineColor = default("black"); // Orientation line color, black by default
        string orientationLineStyle = default("solid"); // Orientation line style (solid, dashed, dotted)
        double orientationLineWidth = default(1); // Orientation line width
        string orientationFillColor = default("blue"); // Orientation fill color, blue by default

        bool displayVelocities = default(false); // Display an arrow indicating the current velocity, disabled by default
        double velocityArrowScale = default(1); // Velocity scaling factor to pixels
        string velocityLineColor = default("black"); // Velocity line color, black by default
        string velocityLineStyle = default("solid"); // Velocity line style (solid, dashed, dotted)
        double velocityLineWidth = default(1); // Velocity line width

        bool displayMovementTrails = default(false); // Display a line along the recent path of mobilities, disabled by default
        string movementTrailLineColor = default("dark"); // Movement trail line color is a list of colors or a color group name (e.g. dark, light), a set of dark colors by default
        string movementTrailLineStyle = default("solid"); // Movement trail line style (solid, dashed, dotted)
        double movementTrailLineWidth = default(1); // Movement trail line width
        int trailLength = default(100); // Number of sections in the trail

        @class(MobilityVisualizerBase);
}
