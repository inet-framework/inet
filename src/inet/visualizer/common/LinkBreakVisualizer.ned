//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.ILinkBreakVisualizer;

//
// This module integrates canvas and osg link break visualizers into a single module.
//
// @see ~LinkBreakCanvasVisualizer, ~LinkBreakOsgVisualizer, ~LinkBreakVisualizerBase, ~ILinkBreakVisualizer
//
module LinkBreakVisualizer like ILinkBreakVisualizer
{
    parameters:
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("LinkBreakCanvasVisualizer"))> like ILinkBreakVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("LinkBreakOsgVisualizer"))> like ILinkBreakVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

