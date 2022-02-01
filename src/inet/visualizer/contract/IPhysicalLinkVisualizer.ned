//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// This module interface allows configuring different ~IPhysicalLinkVisualizer
// submodules from INI files. Such a visualizer displays the active physical
// layer links in a network.
//
// @see ~PhysicalLinkCanvasVisualizer, ~PhysicalLinkOsgVisualizer, ~PhysicalLinkVisualizer, ~PhysicalLinkVisualizerBase
//
moduleinterface IPhysicalLinkVisualizer
{
    parameters:
        @display("i=block/app");
}

