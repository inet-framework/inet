//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.common;

import inet.visualizer.base.PacketDropVisualizerBase;
import inet.visualizer.contract.IPacketDropVisualizer;

//
// Visualizes packet drops on a 3D VulkanSceneGraph scene. Displays a coloured
// sphere for each packet drop in the network; the sphere fades out and drifts
// upward over time according to the fadeOutMode/fadeOutTime parameters. The
// sphere colour is chosen from the iconTintColor set based on the drop reason.
//
// Note: the OSG twin renders a textured packet icon (msg/packet_s); this VSG
// implementation approximates that with a coloured sphere because VsgUtils does
// not yet expose a textured-billboard helper.
//
// @see ~PacketDropCanvasVisualizer, ~PacketDropVisualizer, ~PacketDropVisualizerBase, ~IPacketDropVisualizer
//
simple PacketDropVsgVisualizer extends PacketDropVisualizerBase like IPacketDropVisualizer
{
    parameters:
        @class(PacketDropVsgVisualizer);
}
