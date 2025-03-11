//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.common;

import inet.visualizer.base.PacketDropVisualizerBase;
import inet.visualizer.contract.IPacketDropVisualizer;

//
// Visualizes packet drops on a 2D canvas. Displays animated icons at locations
// where packets are discarded in the network.
//
// The visualization includes:
// - Icons appearing at the exact locations of packet drops
// - Animations showing packets falling to the ground
// - Different icons for various drop reasons (queue overflow, bit error, etc.) TODO no such feature; repeat format string directives?
// - Filtering options to visualize only specific packet types or nodes
//
// Particularly useful for debugging network issues, identifying congestion points,
// understanding protocol behavior under packet loss conditions, and analyzing
// network performance problems.
//
// @see ~PacketDropOsgVisualizer, ~PacketDropVisualizer, ~PacketDropVisualizerBase, ~IPacketDropVisualizer
//
simple PacketDropCanvasVisualizer extends PacketDropVisualizerBase like IPacketDropVisualizer
{
    parameters:
        double zIndex = default(10); // Determines the drawing order of figures relative to other visualizers
        double dx = default(32); // Specifies how far the animation turns over along the X axis
        double dy = default(64); // Specifies how far the animation turns over along the Y axis
        @class(PacketDropCanvasVisualizer);
}
