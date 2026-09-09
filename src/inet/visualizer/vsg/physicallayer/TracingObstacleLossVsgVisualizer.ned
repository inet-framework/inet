//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.physicallayer;

import inet.visualizer.base.TracingObstacleLossVisualizerBase;
import inet.visualizer.contract.ITracingObstacleLossVisualizer;

//
// Visualizes obstacle loss on a 3D VulkanSceneGraph scene. It displays
// a line at the intersections of signal propagation paths with physical objects,
// and optionally the normal vectors of physical object faces at intersection points.
//
// @see ~TracingObstacleLossOsgVisualizer, ~TracingObstacleLossVisualizer, ~TracingObstacleLossVisualizerBase, ~ITracingObstacleLossVisualizer
//
simple TracingObstacleLossVsgVisualizer extends TracingObstacleLossVisualizerBase like ITracingObstacleLossVisualizer
{
    parameters:
        @class(TracingObstacleLossVsgVisualizer);
}
