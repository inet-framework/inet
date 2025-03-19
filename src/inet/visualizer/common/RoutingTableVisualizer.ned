//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IRoutingTableVisualizer;

//
// Integrates canvas and osg routing table visualizers into a single module.
//
// @see ~RoutingTableCanvasVisualizer, ~RoutingTableOsgVisualizer, ~RoutingTableVisualizerBase, ~IRoutingTableVisualizer
//
module RoutingTableVisualizer like IRoutingTableVisualizer
{
    parameters:
        @display("i=block/app");
        string visualizationTargetModule = default("^");
        string visualizationSubjectModule = default("^");
        *.visualizationTargetModule = default(absPath(this.visualizationTargetModule));
        *.visualizationSubjectModule = default(absPath(this.visualizationSubjectModule));
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("RoutingTableCanvasVisualizer"))> like IRoutingTableVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("RoutingTableOsgVisualizer"))> like IRoutingTableVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

