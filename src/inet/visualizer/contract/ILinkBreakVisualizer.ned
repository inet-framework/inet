//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// This module interface allows configuring different ~ILinkBreakVisualizer
// submodules from INI files. Such a visualizer displays the data link breaks
// in a network.
//
// @see ~LinkBreakCanvasVisualizer, ~LinkBreakOsgVisualizer, ~LinkBreakVisualizer, ~LinkBreakVisualizerBase
//
moduleinterface ILinkBreakVisualizer
{
    parameters:
        @display("i=block/app");
}

