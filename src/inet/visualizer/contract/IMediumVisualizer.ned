//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface allows configuring different ~IMediumVisualizer
// submodules from INI files. Such a visualizer displays transmitting nodes,
// receiving nodes, and propagating radio signals on a medium.
//
// @see ~MediumCanvasVisualizer, ~MediumOsgVisualizer, ~MediumVisualizer, ~MediumVisualizerBase
//
moduleinterface IMediumVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

