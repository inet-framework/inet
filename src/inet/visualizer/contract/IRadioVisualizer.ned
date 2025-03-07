//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for radio visualizers. Radio visualizers display the state and
// operating mode of radio modules in network nodes.
//
// @see ~RadioCanvasVisualizer, ~RadioOsgVisualizer, ~RadioVisualizer, ~RadioVisualizerBase
//
moduleinterface IRadioVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}
