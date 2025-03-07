//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.transportlayer;

import inet.visualizer.base.PathVisualizerBase;
import inet.visualizer.contract.ITransportRouteVisualizer;

//
// Visualizes transport layer packet routes in a 3D scene using OpenSceneGraph (OSG).
// Displays the paths that packets take through the network at the transport layer
// (TCP, UDP, SCTP).
//
// The visualization includes:
// - 3D polyline arrows showing active transport routes between nodes
// - Directional indicators showing packet flow from source to destination
// - Separate paths for bidirectional communication
// - Temporal fading as routes become inactive over time
// - Automatic updates when new packets travel along routes
// - Dynamic positioning that follows mobile node movement
//
// A transport route becomes active when a packet travels along a sequence of
// network nodes at the transport layer. The polyline representing this route
// gradually fades unless reinforced by subsequent packets following the same path.
//
// Particularly useful for understanding end-to-end communication patterns,
// analyzing transport protocol behavior, debugging connection issues, and
// creating dynamic visualizations of application traffic flows in 3D space.
//
// @see ~TransportRouteCanvasVisualizer, ~TransportRouteVisualizer, ~PathVisualizerBase, ~ITransportRouteVisualizer
//
simple TransportRouteOsgVisualizer extends PathVisualizerBase like ITransportRouteVisualizer
{
    parameters:
        @class(TransportRouteOsgVisualizer);
}
