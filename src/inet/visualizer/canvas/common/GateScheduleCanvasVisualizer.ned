//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.common;

import inet.visualizer.base.GateScheduleVisualizerBase;
import inet.visualizer.contract.IGateScheduleVisualizer;

//
// Visualizes gate schedules on a 2D canvas. Displays time-based representations
// of gate operations for Time-Sensitive Networking (TSN) and other time-scheduled
// protocols.
//
// The visualization includes:
// - Timeline-style displays showing gate open/close schedules
// - Current time indicator showing the present moment in the schedule
// - Customizable labels identifying gates and interfaces
// - Configurable time window to focus on specific schedule periods
//
// Particularly useful for debugging TSN configurations, analyzing time-synchronized
// network behavior, verifying gate scheduling algorithms.
//
// @see ~GateScheduleOsgVisualizer, ~GateScheduleVisualizer, ~GateScheduleVisualizerBase, ~IGateScheduleVisualizer
//
simple GateScheduleCanvasVisualizer extends GateScheduleVisualizerBase like IGateScheduleVisualizer
{
    parameters:
        double zIndex = default(10); // Determines the drawing order of figures relative to other visualizers
        @class(GateScheduleCanvasVisualizer);
}
