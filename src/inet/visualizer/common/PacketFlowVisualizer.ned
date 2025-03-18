//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.visualizer.common;

import inet.visualizer.contract.IPacketFlowVisualizer;

//
// Visualizes packet flows through the network as animated paths. This compound
// module integrates both canvas (2D) and OSG (3D) visualizers, automatically
// selecting the appropriate one based on the runtime environment. It tracks and
// displays the routes that packets take between network nodes, providing insight
// into network traffic patterns and protocol behavior.
//
module PacketFlowVisualizer like IPacketFlowVisualizer
{
    parameters:
        @display("i=block/app");
        string visualizationTargetModule = default("^");
        string visualizationSubjectModule = default("^");
        *.visualizationTargetModule = default(absPath(this.visualizationTargetModule));
        *.visualizationSubjectModule = default(absPath(this.visualizationSubjectModule));
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("PacketFlowCanvasVisualizer"))> like IPacketFlowVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("PacketFlowOsgVisualizer"))> like IPacketFlowVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

