//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.common;

import inet.visualizer.base.GateScheduleVisualizerBase;
import inet.visualizer.contract.IGateScheduleVisualizer;

//
// Visualizes gates on a 3D VulkanSceneGraph scene. Displays a coloured
// indicator box annotation above each matching gate module: green means the
// gate is currently open, red means it is closed. An optional text label
// shows information formatted by the ~GateScheduleVisualizerBase stringFormat
// parameter.
//
// @see ~GateScheduleCanvasVisualizer, ~GateScheduleVisualizer, ~GateScheduleVisualizerBase, ~IGateScheduleVisualizer
//
simple GateScheduleVsgVisualizer extends GateScheduleVisualizerBase like IGateScheduleVisualizer
{
    parameters:
        @class(GateScheduleVsgVisualizer);
}
