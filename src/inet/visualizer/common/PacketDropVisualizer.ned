//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.common;

import inet.visualizer.contract.IPacketDropVisualizer;

//
// Integrates canvas and osg packet drop visualizers into a single module.
//
// @see ~PacketDropCanvasVisualizer, ~PacketDropOsgVisualizer, ~PacketDropVisualizerBase, ~IPacketDropVisualizer
//
module PacketDropVisualizer like IPacketDropVisualizer
{
    parameters:
        @display("i=block/app");
        string visualizationTargetModule = default("^");
        string visualizationSubjectModule = default("^");
        *.visualizationTargetModule = default(absPath(this.visualizationTargetModule));
        *.visualizationSubjectModule = default(absPath(this.visualizationSubjectModule));
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

