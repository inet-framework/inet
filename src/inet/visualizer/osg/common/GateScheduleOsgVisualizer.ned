//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.common;

import inet.visualizer.base.GateScheduleVisualizerBase;
import inet.visualizer.contract.IGateScheduleVisualizer;

//
// Visualizes gate schedules in a 3D scene using OpenSceneGraph (OSG). Displays
// time-based representations of gate operations for Time-Sensitive Networking (TSN)
// and other time-scheduled protocols.
//
// The visualization includes:
// - 3D timeline-style displays showing gate open/close schedules
// - Current time indicator showing the present moment in the schedule
// - Customizable labels identifying gates and interfaces
// - Configurable time window to focus on specific schedule periods
//
// Particularly useful for debugging TSN configurations in 3D space, analyzing
// time-synchronized network behavior, verifying gate scheduling algorithms, and
// understanding deterministic networking operations in a realistic 3D environment.
//
// @see ~GateScheduleCanvasVisualizer, ~GateScheduleVisualizer, ~GateScheduleVisualizerBase, ~IGateScheduleVisualizer
//
simple GateScheduleOsgVisualizer extends GateScheduleVisualizerBase like IGateScheduleVisualizer
{
    parameters:
        @class(GateScheduleOsgVisualizer);
}
