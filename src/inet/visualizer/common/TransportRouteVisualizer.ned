//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.ITransportRouteVisualizer;

//
// This module integrates canvas and osg transport route visualizers into a single module.
//
// @see ~TransportRouteCanvasVisualizer, ~TransportRouteOsgVisualizer, ~PathVisualizerBase, ~ITransportRouteVisualizer
//
module TransportRouteVisualizer like ITransportRouteVisualizer
{
    parameters:
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("TransportRouteCanvasVisualizer"))> like ITransportRouteVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("TransportRouteOsgVisualizer"))> like ITransportRouteVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

