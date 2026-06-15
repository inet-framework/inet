//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.networklayer;

import inet.visualizer.base.RoutingTableVisualizerBase;
import inet.visualizer.contract.IRoutingTableVisualizer;

//
// Visualizes routes in the routing tables on a 3D VulkanSceneGraph scene. It
// displays an arrow for each route in the routing tables of the network. The
// arrow points from the node containing the routing table towards the gateway.
// The arrow automatically follows the movement of mobile nodes.
//
// @see ~RoutingTableCanvasVisualizer, ~RoutingTableOsgVisualizer, ~RoutingTableVisualizer, ~RoutingTableVisualizerBase, ~IRoutingTableVisualizer
//
simple RoutingTableVsgVisualizer extends RoutingTableVisualizerBase like IRoutingTableVisualizer
{
    parameters:
        @class(RoutingTableVsgVisualizer);
}
