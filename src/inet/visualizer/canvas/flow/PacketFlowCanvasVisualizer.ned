//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.flow;

import inet.visualizer.base.PathVisualizerBase;
import inet.visualizer.contract.IPacketFlowVisualizer;

// TODO
// Visualizes packet flows on a 2D canvas. Displays dashed arrows representing
// the complete path of packet flows through the network.
//
// The visualization includes:
// - Dashed arrows showing the path from source to destination
// - Labels indicating packet counts and flow information
// - Paths that persist rather than fade out (by default)
// - Filtering options to visualize only specific packet types or nodes
//
// Particularly useful for understanding overall traffic patterns, analyzing
// application data flows, and visualizing the complete path of packets through
// multiple protocol layers.
//
// @see ~PacketFlowOsgVisualizer, ~PacketFlowVisualizer, ~PathVisualizerBase, ~IPacketFlowVisualizer
//
simple PacketFlowCanvasVisualizer extends PathVisualizerBase like IPacketFlowVisualizer
{
    parameters:
        startPathSignal = default("packetFlowStarted");
        endPathSignal = default("packetFlowEnded");
        labelFormat = default("%L %p pk (%l)");
        lineStyle = default("dashed");
        fadeOutTime = default(0s);
        double zIndex = default(4); // Determines the drawing order of figures relative to other visualizers
        @class(PacketFlowCanvasVisualizer);
}
