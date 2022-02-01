//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.physicallayer;

import inet.visualizer.base.TracingObstacleLossVisualizerBase;
import inet.visualizer.contract.ITracingObstacleLossVisualizer;

//
// This module visualizes obstacle loss on a 3D osg scene. It displays a line at
// the intersections of signal propagation paths with physical objects, and
// normal vectors of physical object faces at intersection points.
//
// @see ~TracingObstacleLossOsgVisualizer, ~TracingObstacleLossVisualizer, ~TracingObstacleLossVisualizerBase, ~ITracingObstacleLossVisualizer
//
simple TracingObstacleLossOsgVisualizer extends TracingObstacleLossVisualizerBase like ITracingObstacleLossVisualizer
{
    parameters:
        @class(TracingObstacleLossOsgVisualizer);
}

