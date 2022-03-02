//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.common;

import inet.visualizer.base.GateScheduleVisualizerBase;
import inet.visualizer.contract.IGateScheduleVisualizer;

//
// This module visualizes gates on a 2D canvas.
//
// @see ~GateScheduleOsgVisualizer, ~GateScheduleVisualizer, ~GateScheduleVisualizerBase, ~IGateScheduleVisualizer
//
simple GateScheduleCanvasVisualizer extends GateScheduleVisualizerBase like IGateScheduleVisualizer
{
    parameters:
        double zIndex = default(10); // determines the drawing order of figures relative to other visualizers
        @class(GateScheduleCanvasVisualizer);
}

