//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.common;

import inet.visualizer.base.QueueVisualizerBase;
import inet.visualizer.contract.IQueueVisualizer;

//
// This module visualizes queues on a 3D osg scene.
//
// @see ~QueueCanvasVisualizer, ~QueueVisualizer, ~QueueVisualizerBase, ~IQueueVisualizer
//
simple QueueOsgVisualizer extends QueueVisualizerBase like IQueueVisualizer
{
    parameters:
        @class(QueueOsgVisualizer);
}

