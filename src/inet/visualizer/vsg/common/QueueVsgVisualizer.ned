//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.common;

import inet.visualizer.base.QueueVisualizerBase;
import inet.visualizer.contract.IQueueVisualizer;

//
// Visualizes queues on a 3D VulkanSceneGraph scene. Displays a filled bar
// annotation above each matching queue module; the bar height reflects the
// queue fill ratio (green = empty, red = full) and a text label shows the
// current packet count.
//
// @see ~QueueCanvasVisualizer, ~QueueVisualizer, ~QueueVisualizerBase, ~IQueueVisualizer
//
simple QueueVsgVisualizer extends QueueVisualizerBase like IQueueVisualizer
{
    parameters:
        @class(QueueVsgVisualizer);
}
