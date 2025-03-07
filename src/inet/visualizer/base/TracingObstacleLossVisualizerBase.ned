//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for tracing obstacle loss visualizer simple modules. It
// visualizes how wireless signals interact with physical objects in the environment,
// showing the points where signals penetrate through walls, buildings, and other obstacles.
//
// By default, intersections are not displayed. To enable visualization, set the
// displayIntersections parameter to true. You can also enable the display of face
// normal vectors at intersection points by setting displayFaceNormalVectors to true.
//
// The visualizer shows the intersection points between signal paths and physical
// objects as red lines, with optional grey normal vectors indicating the orientation
// of the penetrated surface. These visualizations fade out after a configurable
// amount of time.
//
// This visualizer is particularly useful for understanding signal propagation in
// complex environments, analyzing the impact of obstacles on wireless communication,
// and debugging obstacle loss models that calculate signal attenuation through
// different materials.
//
// The actual rendering of obstacle loss visualizations is implemented in derived
// modules for specific user interfaces (canvas or OSG).
//
// @see ~TracingObstacleLossCanvasVisualizer, ~TracingObstacleLossOsgVisualizer, ~ITracingObstacleLossVisualizer, ~VisualizerBase
//
simple TracingObstacleLossVisualizerBase extends VisualizerBase
{
    parameters:
        bool displayIntersections = default(false); // Display intersections of transmissions and physical objects, disabled by default
        string intersectionLineColor = default("red");
        string intersectionLineStyle = default("solid");
        double intersectionLineWidth = default(1);

        bool displayFaceNormalVectors = default(false); // Display face normal vectors of intersections, disabled by default
        string faceNormalLineColor = default("grey");
        string faceNormalLineStyle = default("solid");
        double faceNormalLineWidth = default(1);

        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // Specifies how inactive obstacle losses fade out
        double fadeOutTime @unit(s) = default(1s); // How quickly link breaks fade away, 1 second by default
        double fadeOutAnimationSpeed = default(0); // Animation speed while there are obstacle losses, value must be in the range (0, +inf)

        @class(TracingObstacleLossVisualizerBase);
}
