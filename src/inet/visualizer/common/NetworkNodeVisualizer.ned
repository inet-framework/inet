//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.INetworkNodeVisualizer;

//
// This module integrates canvas and osg node visualizers into a single module.
//
// @see ~NetworkNodeCanvasVisualizer, ~NetworkNodeOsgVisualizer, ~NetworkNodeVisualizerBase, ~INetworkNodeVisualizer
//
module NetworkNodeVisualizer like INetworkNodeVisualizer
{
    parameters:
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("NetworkNodeCanvasVisualizer"))> like INetworkNodeVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("NetworkNodeOsgVisualizer"))> like INetworkNodeVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

