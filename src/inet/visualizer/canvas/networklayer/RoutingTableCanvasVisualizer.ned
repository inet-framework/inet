//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.networklayer;

import inet.visualizer.base.RoutingTableVisualizerBase;
import inet.visualizer.contract.IRoutingTableVisualizer;

//
// This module visualizes routes in the routing tables on a 2D canvas. It displays
// an arrow for each route in the routing tables of the network. The arrow points
// from the node containing the routing table towards the gateway. The arrow
// automatically follows the movement of mobile nodes.
//
// @see ~RoutingTableOsgVisualizer, ~RoutingTableVisualizer, ~RoutingTableVisualizerBase, ~IRoutingTableVisualizer
//
simple RoutingTableCanvasVisualizer extends RoutingTableVisualizerBase like IRoutingTableVisualizer
{
    parameters:
        double zIndex = default(0); // determines the drawing order of figures relative to other visualizers
        @class(RoutingTableCanvasVisualizer);
}

