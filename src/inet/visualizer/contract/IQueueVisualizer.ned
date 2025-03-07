//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for queue visualizers. Queue visualizers display the state and
// occupancy of packet queues in network nodes.
//
// @see ~QueueCanvasVisualizer, ~QueueOsgVisualizer, ~QueueVisualizer, ~QueueVisualizerBase
//
moduleinterface IQueueVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}
