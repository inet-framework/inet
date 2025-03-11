//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.transportlayer;

import inet.visualizer.base.TransportConnectionVisualizerBase;
import inet.visualizer.contract.ITransportConnectionVisualizer;

//
// Visualizes transport layer connections in a 3D scene using OpenSceneGraph (OSG).
// Displays indicators for active TCP, UDP, and SCTP connections between network nodes.
//
// The visualization includes:
// - Colored icons at both endpoints of each connection
// - Different colors for different connection types or states
// - Automatic updates when connections are established or closed
// - Filtering options to visualize only specific connections or protocols
// - Support for TCP, UDP, SCTP and other transport protocols
//
// Each transport connection is represented by a pair of icons at the source
// and destination nodes, making it easy to identify which nodes are communicating
// with each other at the transport layer.
//
// Particularly useful for understanding application communication patterns,
// analyzing transport protocol usage, debugging connection establishment issues,
// and creating visual representations of end-to-end connectivity in 3D view.
//
// @see ~TransportConnectionCanvasVisualizer, ~TransportConnectionVisualizer, ~TransportConnectionVisualizerBase, ~ITransportConnectionVisualizer
//
simple TransportConnectionOsgVisualizer extends TransportConnectionVisualizerBase like ITransportConnectionVisualizer
{
    parameters:
        @class(TransportConnectionOsgVisualizer);
}
