//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// This module interface allows configuring different ~IInfoVisualizer
// submodules from INI files. Such a visualizer displays various information
// from submodules of network nodes.
//
// @see ~InfoCanvasVisualizer, ~InfoOsgVisualizer, ~InfoVisualizer, ~InfoVisualizerBase
//
moduleinterface IInfoVisualizer
{
    parameters:
        @display("i=block/app");
}

