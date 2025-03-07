//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.physicallayer;

import inet.visualizer.base.ChannelVisualizerBase;
import inet.visualizer.contract.IChannelVisualizer;

//
// Visualizes channel activity on a 2D canvas. Displays arrows representing
// active communication channels between network nodes.
//
// The visualization includes:
// - Arrows indicating active channel connections between nodes
// - Color-coded arrows with customizable appearance
// - Gradual fading of arrows as channels become inactive
// - Labels showing channel information
// - Filtering options to visualize only specific packet types or nodes
//
// Particularly useful for understanding channel utilization patterns,
// analyzing communication flows, and troubleshooting channel-related issues.
//
// @see ~ChannelOsgVisualizer, ~ChannelVisualizer, ~ChannelVisualizerBase, ~IChannelVisualizer
//
simple ChannelCanvasVisualizer extends ChannelVisualizerBase like IChannelVisualizer
{
    parameters:
        double zIndex = default(1); // Determines the drawing order of figures relative to other visualizers.
        @class(ChannelCanvasVisualizer);
}
