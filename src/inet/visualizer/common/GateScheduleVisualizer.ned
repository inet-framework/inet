//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IGateScheduleVisualizer;

//
// This module integrates canvas and osg gate visualizers into a single module.
//
// @see ~GateScheduleCanvasVisualizer, ~GateScheduleOsgVisualizer, ~GateScheduleVisualizerBase, ~IGateScheduleVisualizer
//
module GateScheduleVisualizer like IGateScheduleVisualizer
{
    parameters:
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
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

