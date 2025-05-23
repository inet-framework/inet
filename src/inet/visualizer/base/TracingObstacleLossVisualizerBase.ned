//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// Base module for tracing obstacle loss visualizer simple modules. It
// subscribes to the obstacle loss module for physical object penetrated signals.
// The actual visualization of obstacle loss is done in derived modules.
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

