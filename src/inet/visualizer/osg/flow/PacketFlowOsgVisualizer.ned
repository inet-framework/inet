//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.visualizer.osg.flow;

import inet.visualizer.base.PathVisualizerBase;
import inet.visualizer.contract.IPacketFlowVisualizer;

//
// Visualizes packet flows through the network using 3D graphics with OSG
// (OpenSceneGraph). Displays animated paths that packets take as they traverse
// the network, with customizable line styles, colors, and labels. Tracks flows
// through network elements like Ethernet switches and IEEE 802.1d bridges,
// helping to understand network traffic patterns and routing behavior.
//
simple PacketFlowOsgVisualizer extends PathVisualizerBase like IPacketFlowVisualizer
{
    parameters:
        startPathSignal = default("packetFlowStarted");
        endPathSignal = default("packetFlowEnded");
        labelFormat = default("%L %p pk (%l)");
        lineStyle = default("dashed");
        fadeOutTime = default(0s);
        @class(PacketFlowOsgVisualizer);
}

