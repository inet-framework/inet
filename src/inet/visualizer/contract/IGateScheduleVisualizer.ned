//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for gate schedule visualizers. Gate schedule visualizers display
// the state and timing of gates in time-aware network devices.
//
// @see ~GateScheduleCanvasVisualizer, ~GateScheduleOsgVisualizer, ~GateScheduleVisualizer, ~GateScheduleVisualizerBase
//
moduleinterface IGateScheduleVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}
