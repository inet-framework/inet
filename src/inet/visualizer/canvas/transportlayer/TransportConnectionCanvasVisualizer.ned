//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.transportlayer;

import inet.visualizer.base.TransportConnectionVisualizerBase;
import inet.visualizer.contract.ITransportConnectionVisualizer;

//
// Visualizes transport layer connections on a 2D canvas. Displays colored icons
// at network nodes to represent active transport connections (TCP, UDP, etc.).
//
// The visualization includes:
// - Pairs of icons at source and destination nodes for each connection
// - Color coding to distinguish different connections
// - Automatic updates when connections are established or closed
// - Filtering options to show only specific connections or nodes
//
// Particularly useful for understanding connection patterns in client-server
// applications, tracking connection establishment, and debugging transport
// layer issues.
//
// @see ~TransportConnectionOsgVisualizer, ~TransportConnectionVisualizer, ~TransportConnectionVisualizerBase, ~ITransportConnectionVisualizer
//
simple TransportConnectionCanvasVisualizer extends TransportConnectionVisualizerBase like ITransportConnectionVisualizer
{
    parameters:
        double zIndex = default(4); // Determines the drawing order of figures relative to other visualizers
        @class(TransportConnectionCanvasVisualizer);
}
