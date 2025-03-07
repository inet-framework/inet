
//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for queue visualizer simple modules. It visualizes
// packet queues in network nodes, showing their current
// fill level.
//
// By default, queues are not displayed. To enable visualization, set the
// displayQueues parameter to true and use the queueFilter parameter to select which
// queues to visualize.
//
// The visualizer shows queues as vertical bars attached to network nodes,
// with the height of the colored portion representing the current queue
// utilization (number of packets or bytes relative to capacity). This helps
// identify congestion points in the network where packets are being buffered.
//
// This visualizer is particularly useful for understanding queueing behavior,
// analyzing bottlenecks in the network, debugging congestion control mechanisms,
// and observing the effects of different queue management algorithms.
//
// The actual rendering of queue indicators is implemented in derived modules
// for specific user interfaces (canvas or OSG).
//
// @see ~QueueCanvasVisualizer, ~QueueOsgVisualizer, ~IQueueVisualizer, ~VisualizerBase
//
simple QueueVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // Module path to the network node visualizer module, refers to a sibling submodule by default

        bool displayQueues = default(false);

        string queueFilter = default("**"); // Which queues are considered, matches all queues by default

        string color = default("blue");
        double width = default(8);
        double height = default(24);
        double spacing = default(2);
        string placementHint = default("right"); // Annotation placement hint, space-separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // Determines the order of annotation positioning

        @class(QueueVisualizerBase);
}
