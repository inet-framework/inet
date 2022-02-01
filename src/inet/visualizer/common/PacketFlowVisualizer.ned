//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IPacketFlowVisualizer;

module PacketFlowVisualizer like IPacketFlowVisualizer
{
    parameters:
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
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

