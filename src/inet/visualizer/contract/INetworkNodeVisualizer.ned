//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface allows configuring different ~INetworkNodeVisualizer
// submodules from INI files. Such a visualizer creates the visual representations
// of network nodes.
//
// @see ~NetworkNodeCanvasVisualizer, ~NetworkNodeOsgVisualizer, ~NetworkNodeVisualizer, ~NetworkNodeVisualizerBase
//
moduleinterface INetworkNodeVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

