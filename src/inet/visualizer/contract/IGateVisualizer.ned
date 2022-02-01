//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// This module interface allows configuring different ~IGateVisualizer
// submodules from INI files. Such a visualizer displays gates.
//
// @see ~GateCanvasVisualizer, ~GateOsgVisualizer, ~GateVisualizer, ~GateVisualizerBase
//
moduleinterface IGateVisualizer
{
    parameters:
        @display("i=block/app");
}

