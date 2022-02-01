//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.flow;

import inet.visualizer.base.PathVisualizerBase;
import inet.visualizer.contract.IPacketFlowVisualizer;

simple PacketFlowCanvasVisualizer extends PathVisualizerBase like IPacketFlowVisualizer
{
    parameters:
        startPathSignal = default("packetFlowStarted");
        endPathSignal = default("packetFlowEnded");
        labelFormat = default("%L %p pk (%l)");
        lineStyle = default("dashed");
        fadeOutTime = default(0s);
        double zIndex = default(4); // determines the drawing order of figures relative to other visualizers
        @class(PacketFlowCanvasVisualizer);
}

