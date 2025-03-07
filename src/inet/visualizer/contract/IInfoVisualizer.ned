//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for info visualizers. Info visualizers display textual information
// about network nodes and their submodules.
//
// @see ~InfoCanvasVisualizer, ~InfoOsgVisualizer, ~InfoVisualizer, ~InfoVisualizerBase
//
moduleinterface IInfoVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}
