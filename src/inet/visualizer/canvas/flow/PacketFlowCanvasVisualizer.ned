//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.visualizer.canvas.flow;

import inet.visualizer.base.PathVisualizerBase;
import inet.visualizer.contract.IPacketFlowVisualizer;

//
// Visualizes packet flows through the network as animated paths on a canvas.
// Tracks and displays the routes that packets take between network nodes by
// drawing polyline arrows. Uses FlowTag to identify and follow packet flows,
// supporting various network elements like MacRelayUnit, Ieee8021dRelay, and
// Ipv4. Provides customization options for line style, color, labels, and
// animation. Useful for debugging network simulations, understanding packet flow
// patterns, and analyzing network behavior through visual representation.
//
simple PacketFlowCanvasVisualizer extends PathVisualizerBase like IPacketFlowVisualizer
{
    parameters:
        startPathSignal = default("packetFlowStarted");
        endPathSignal = default("packetFlowEnded");
        labelFormat = default("%L %p pk (%l)");
        lineStyle = default("dashed");
        fadeOutTime = default(0s);
        double zIndex = default(4); // Determines the drawing order of figures relative to other visualizers
        @class(PacketFlowCanvasVisualizer);
}

