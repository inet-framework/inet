//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IQueueVisualizer;

//
// Integrates canvas and osg queue visualizers into a single module.
//
// @see ~QueueCanvasVisualizer, ~QueueOsgVisualizer, ~QueueVisualizerBase, ~IQueueVisualizer
//
module QueueVisualizer like IQueueVisualizer
{
    parameters:
        @display("i=block/app");
        string visualizationTargetModule = default("^");
        string visualizationSubjectModule = default("^");
        *.visualizationTargetModule = default(absPath(this.visualizationTargetModule));
        *.visualizationSubjectModule = default(absPath(this.visualizationSubjectModule));
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("QueueCanvasVisualizer"))> like IQueueVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("QueueOsgVisualizer"))> like IQueueVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

