//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IIntegratedVisualizer;

//
// This module integrates canvas and osg visualizers into a single module.
//
module IntegratedVisualizer like IIntegratedVisualizer
{
    parameters:
        @display("i=block/app");
        *.*.visualizationTargetModule = default("^.^.^");
        *.*.visualizationSubjectModule = default("^.^.^");
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("IntegratedOsgVisualizer"))> like IIntegratedVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

