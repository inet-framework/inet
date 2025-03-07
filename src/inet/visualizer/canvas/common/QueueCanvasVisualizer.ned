//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.common;

import inet.visualizer.base.QueueVisualizerBase;
import inet.visualizer.contract.IQueueVisualizer;

//
// Visualizes packet queues on a 2D canvas. Displays indicators showing
// queue occupancy and status in network nodes.
//
// The visualization includes:
// - Colored bars representing the current queue fill levels
// - Customizable size, color, and placement of queue indicators
// - Automatic updates as queue occupancy changes during simulation
// - Filtering options to visualize only specific queues
//
// Particularly useful for understanding buffer usage patterns, identifying
// potential bottlenecks, analyzing congestion points, and debugging
// queueing-related issues in the network.
//
// @see ~QueueOsgVisualizer, ~QueueVisualizer, ~QueueVisualizerBase, ~IQueueVisualizer
//
simple QueueCanvasVisualizer extends QueueVisualizerBase like IQueueVisualizer
{
    parameters:
        double zIndex = default(10); // Determines the drawing order of figures relative to other visualizers
        @class(QueueCanvasVisualizer);
}
