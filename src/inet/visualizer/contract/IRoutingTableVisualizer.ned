//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for routing table visualizers. Routing table visualizers display
// the routes stored in the routing tables of network nodes.
//
// @see ~RoutingTableCanvasVisualizer, ~RoutingTableOsgVisualizer, ~RoutingTableVisualizer, ~RoutingTableVisualizerBase
//
moduleinterface IRoutingTableVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}
