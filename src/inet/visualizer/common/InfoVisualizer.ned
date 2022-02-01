//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IInfoVisualizer;

//
// This module integrates canvas and osg info visualizers into a single module.
//
// @see ~InfoCanvasVisualizer, ~InfoOsgVisualizer, ~InfoVisualizerBase, ~IInfoVisualizer
//
module InfoVisualizer like IInfoVisualizer
{
    parameters:
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("InfoCanvasVisualizer"))> like IInfoVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("InfoOsgVisualizer"))> like IInfoVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}
