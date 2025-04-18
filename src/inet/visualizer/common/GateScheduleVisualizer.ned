//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IGateScheduleVisualizer;

//
// Integrates canvas and osg gate visualizers into a single module.
//
// @see ~GateScheduleCanvasVisualizer, ~GateScheduleOsgVisualizer, ~GateScheduleVisualizerBase, ~IGateScheduleVisualizer
//
module GateScheduleVisualizer like IGateScheduleVisualizer
{
    parameters:
        @display("i=block/app");
        string visualizationTargetModule = default("^");
        string visualizationSubjectModule = default("^");
        *.visualizationTargetModule = default(absPath(this.visualizationTargetModule));
        *.visualizationSubjectModule = default(absPath(this.visualizationSubjectModule));
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("GateScheduleCanvasVisualizer"))> like IGateScheduleVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("GateScheduleOsgVisualizer"))> like IGateScheduleVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

