//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// This module interface allows configuring different ~IGateScheduleVisualizer
// submodules from INI files. Such a visualizer displays gates.
//
// @see ~GateScheduleCanvasVisualizer, ~GateScheduleOsgVisualizer, ~GateScheduleVisualizer, ~GateScheduleVisualizerBase
//
moduleinterface IGateScheduleVisualizer
{
    parameters:
        @display("i=block/app");
}

