//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.flow;

import inet.visualizer.base.PathVisualizerBase;
import inet.visualizer.contract.IPacketFlowVisualizer;

//
// Visualizes packet flows in a 3D scene using OpenSceneGraph (OSG). Displays
// the complete path that packets take through the network across all protocol layers.
//
// The visualization includes:
// - 3D polyline arrows showing end-to-end packet flows between nodes
// - Labels indicating packet counts and flow information
// - Dashed lines distinguishing flows from other path visualizations
// - Customizable appearance including colors and line styles
// - Filtering options to visualize only specific packet flows
//
// A packet flow represents the complete journey of packets from source to destination,
// potentially spanning multiple protocol layers and network nodes. This visualizer
// tracks packets using flow signals emitted during packet processing.
//
// Particularly useful for understanding complex network behavior, analyzing
// cross-layer packet processing, debugging packet flow issues, and creating
// comprehensive visualizations of network traffic in 3D space.
//
// @see ~PacketFlowCanvasVisualizer, ~PacketFlowVisualizer, ~PathVisualizerBase, ~IPacketFlowVisualizer
//
simple PacketFlowOsgVisualizer extends PathVisualizerBase like IPacketFlowVisualizer
{
    parameters:
        startPathSignal = default("packetFlowStarted");
        endPathSignal = default("packetFlowEnded");
        labelFormat = default("%L %p pk (%l)");
        lineStyle = default("dashed");
        fadeOutTime = default(0s);
        @class(PacketFlowOsgVisualizer);
}
