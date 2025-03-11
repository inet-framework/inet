//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.networklayer;

import inet.visualizer.base.RoutingTableVisualizerBase;
import inet.visualizer.contract.IRoutingTableVisualizer;

//
// Visualizes routing table entries in a 3D scene using OpenSceneGraph (OSG).
// Displays the next-hop relationships between network nodes based on their
// routing tables.
//
// The visualization includes:
// - 3D arrows showing routes from nodes to their gateways
// - Directional indicators showing routing paths through the network
// - Filtering options to visualize only specific routes or nodes
// - Dynamic positioning that follows mobile node movement
// - Support for various routing protocols (static routes, OSPF, RIP, BGP, etc.)
//
// Each arrow represents a routing table entry, pointing from the node that
// contains the entry toward the gateway node that would be used to reach
// the destination.
//
// Particularly useful for understanding routing topologies, analyzing routing
// protocol behavior, debugging routing configurations, and creating visual
// representations of network routing structures in 3D view.
//
// @see ~RoutingTableCanvasVisualizer, ~RoutingTableVisualizer, ~RoutingTableVisualizerBase, ~IRoutingTableVisualizer
//
simple RoutingTableOsgVisualizer extends RoutingTableVisualizerBase like IRoutingTableVisualizer
{
    parameters:
        @class(RoutingTableOsgVisualizer);
}
