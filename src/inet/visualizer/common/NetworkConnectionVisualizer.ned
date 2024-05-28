//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.INetworkConnectionVisualizer;

//
// This module integrates canvas and OSG connection visualizers into a single module.
//
// @see ~NetworkConnectionCanvasVisualizer, ~NetworkConnectionOsgVisualizer, ~NetworkConnectionVisualizerBase, ~INetworkConnectionVisualizer
//
module NetworkConnectionVisualizer like INetworkConnectionVisualizer
{
    parameters:
        @display("i=block/app");
        string visualizationTargetModule = default("^");
        string visualizationSubjectModule = default("^");
        *.visualizationTargetModule = default(absPath(this.visualizationTargetModule));
        *.visualizationSubjectModule = default(absPath(this.visualizationSubjectModule));
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("NetworkConnectionCanvasVisualizer"))> like INetworkConnectionVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("NetworkConnectionOsgVisualizer"))> like INetworkConnectionVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

