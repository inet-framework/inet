//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.physicallayer;

import inet.visualizer.base.TracingObstacleLossVisualizerBase;
import inet.visualizer.contract.ITracingObstacleLossVisualizer;

//
// Visualizes obstacle loss effects on a 2D canvas. Displays the interaction between
// radio signals and physical objects in the environment.
//
// The visualization includes:
// - Lines at intersection points where signals pass through physical objects
// - Normal vectors showing the orientation of object faces at intersection points
// - Visual indication of signal attenuation due to obstacles
// - Dynamic updates as signals propagate through the environment
//
// Particularly useful for understanding signal propagation in environments with
// obstacles, analyzing coverage problems due to physical barriers, and verifying
// the correct operation of obstacle loss models.
//
// @see ~TracingObstacleLossOsgVisualizer, ~TracingObstacleLossVisualizer, ~TracingObstacleLossVisualizerBase, ~ITracingObstacleLossVisualizer
//
simple TracingObstacleLossCanvasVisualizer extends TracingObstacleLossVisualizerBase like ITracingObstacleLossVisualizer
{
    parameters:
        double zIndex = default(0); // Determines the drawing order of figures relative to other visualizers
        @class(TracingObstacleLossCanvasVisualizer);
}
