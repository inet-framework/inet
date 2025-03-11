//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for physical link visualizers. Physical link visualizers display
// active connections between network interfaces at the
// physical layer.
//
// @see ~PhysicalLinkCanvasVisualizer, ~PhysicalLinkOsgVisualizer, ~PhysicalLinkVisualizer, ~PhysicalLinkVisualizerBase
//
moduleinterface IPhysicalLinkVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}
