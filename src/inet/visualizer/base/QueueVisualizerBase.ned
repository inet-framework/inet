//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for queue visualizer simple modules.
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
        string placementHint = default("right"); // Annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // Determines the order of annotation positioning

        @class(QueueVisualizerBase);
}

