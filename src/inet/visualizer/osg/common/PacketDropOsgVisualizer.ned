//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.common;

import inet.visualizer.base.PacketDropVisualizerBase;
import inet.visualizer.contract.IPacketDropVisualizer;

//
// Visualizes packet drops in a 3D scene using OpenSceneGraph (OSG). Displays
// indicators at locations where packets are discarded in the network.
//
// The visualization includes:
// - 3D icons showing where packets are dropped
// - Different icons for different packet drop reasons
// - Automatic updates when new packet drops occur
// - Filtering options to visualize only specific drop causes or nodes
// - Support for various drop reasons (queue overflow, address resolution failure, etc.)
//
// A packet drop occurs when a network node explicitly discards a packet for reasons
// such as queue overflow, retry limit exceeded, address resolution failure, or
// network congestion.
//
// Particularly useful for debugging network issues, identifying bottlenecks,
// analyzing protocol behavior under stress, and creating visual representations
// of network performance problems in 3D space.
//
// @see ~PacketDropCanvasVisualizer, ~PacketDropVisualizer, ~PacketDropVisualizerBase, ~IPacketDropVisualizer
//
simple PacketDropOsgVisualizer extends PacketDropVisualizerBase like IPacketDropVisualizer
{
    parameters:
        @class(PacketDropOsgVisualizer);
}
