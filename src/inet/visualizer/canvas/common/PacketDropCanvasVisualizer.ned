//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.common;

import inet.visualizer.base.PacketDropVisualizerBase;
import inet.visualizer.contract.IPacketDropVisualizer;

//
// This module visualizes packet drops on a 2D canvas. It displays an icon for
// each packet drop in the network. See the documentation of the base module for
// what constitutes a packet drop.
//
// @see ~PacketDropOsgVisualizer, ~PacketDropVisualizer, ~PacketDropVisualizerBase, ~IPacketDropVisualizer
//
simple PacketDropCanvasVisualizer extends PacketDropVisualizerBase like IPacketDropVisualizer
{
    parameters:
        double zIndex = default(10); // determines the drawing order of figures relative to other visualizers
        double dx = default(32); // specifies how far the animation turns over along the X axis
        double dy = default(64); // specifies how far the animation turns over along the Y axis
        @class(PacketDropCanvasVisualizer);
}

