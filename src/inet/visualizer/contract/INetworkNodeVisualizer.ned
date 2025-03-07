//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for network node visualizers. Network node visualizers create
// visual representations of network nodes (routers, hosts, etc.) in the
// simulation.
//
// @see ~NetworkNodeCanvasVisualizer, ~NetworkNodeOsgVisualizer, ~NetworkNodeVisualizer, ~NetworkNodeVisualizerBase
//
moduleinterface INetworkNodeVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}
