//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IGateVisualizer;

//
// This module integrates canvas and osg gate visualizers into a single module.
//
// @see ~GateCanvasVisualizer, ~GateOsgVisualizer, ~GateVisualizerBase, ~IGateVisualizer
//
module GateVisualizer like IGateVisualizer
{
    parameters:
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("GateCanvasVisualizer"))> like IGateVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("GateOsgVisualizer"))> like IGateVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

