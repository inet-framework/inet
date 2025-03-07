//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for link break visualizers. Link break visualizers display
// indicators at locations where data link connections fail during simulation.
//
// @see ~LinkBreakCanvasVisualizer, ~LinkBreakOsgVisualizer, ~LinkBreakVisualizer, ~LinkBreakVisualizerBase
//
moduleinterface ILinkBreakVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}
