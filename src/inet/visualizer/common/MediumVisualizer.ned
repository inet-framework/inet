//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IMediumVisualizer;

//
// This module integrates canvas and osg medium visualizers into a single module.
//
// @see ~MediumCanvasVisualizer, ~MediumOsgVisualizer, ~MediumVisualizerBase, ~IMediumVisualizer
//
module MediumVisualizer like IMediumVisualizer
{
    parameters:
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("MediumCanvasVisualizer"))> like IMediumVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("MediumOsgVisualizer"))> like IMediumVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

