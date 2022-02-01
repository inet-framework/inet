//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IMobilityVisualizer;

//
// This module integrates canvas and osg mobility visualizers into a single module.
//
// @see ~MobilityCanvasVisualizer, ~MobilityOsgVisualizer, ~MobilityVisualizerBase, ~IMobilityVisualizer
//
module MobilityVisualizer like IMobilityVisualizer
{
    parameters:
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("MobilityCanvasVisualizer"))> like IMobilityVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("MobilityOsgVisualizer"))> like IMobilityVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

