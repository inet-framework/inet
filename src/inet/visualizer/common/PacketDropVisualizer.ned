//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IPacketDropVisualizer;

//
// This module integrates canvas and osg packet drop visualizers into a single module.
//
// @see ~PacketDropCanvasVisualizer, ~PacketDropOsgVisualizer, ~PacketDropVisualizerBase, ~IPacketDropVisualizer
//
module PacketDropVisualizer like IPacketDropVisualizer
{
    parameters:
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
    submodules:
        canvasVisualizer: <default(firstAvailableOrEmpty("PacketDropCanvasVisualizer"))> like IPacketDropVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default(firstAvailableOrEmpty("PacketDropOsgVisualizer"))> like IPacketDropVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

