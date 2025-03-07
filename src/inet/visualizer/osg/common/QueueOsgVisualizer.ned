//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.common;

import inet.visualizer.base.QueueVisualizerBase;
import inet.visualizer.contract.IQueueVisualizer;

//
// Visualizes packet queues in a 3D scene using OpenSceneGraph (OSG). Displays
// indicators showing queue occupancy and status in network nodes.
//
// The visualization includes:
// - 3D bars representing the current queue fill levels
// - Customizable size, color, and placement of queue indicators
// - Automatic updates as queue occupancy changes during simulation
// - Filtering options to visualize only specific queues
//
// Particularly useful for understanding buffer usage patterns, identifying
// potential bottlenecks, analyzing congestion points, debugging
// queueing-related issues, and creating visual representations of network
// buffering behavior in 3D space.
//
// @see ~QueueCanvasVisualizer, ~QueueVisualizer, ~QueueVisualizerBase, ~IQueueVisualizer
//
simple QueueOsgVisualizer extends QueueVisualizerBase like IQueueVisualizer
{
    parameters:
        @class(QueueOsgVisualizer);
}
