//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.common;

import inet.visualizer.base.GateScheduleVisualizerBase;
import inet.visualizer.contract.IGateScheduleVisualizer;

//
// This module visualizes gates on a 3D osg scene.
//
// @see ~GateScheduleCanvasVisualizer, ~GateScheduleVisualizer, ~GateScheduleVisualizerBase, ~IGateScheduleVisualizer
//
simple GateScheduleOsgVisualizer extends GateScheduleVisualizerBase like IGateScheduleVisualizer
{
    parameters:
        @class(GateScheduleOsgVisualizer);
}

