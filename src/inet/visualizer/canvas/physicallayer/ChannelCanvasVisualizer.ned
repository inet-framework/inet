//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.visualizer.canvas.physicallayer;

import inet.visualizer.base.ChannelVisualizerBase;
import inet.visualizer.contract.IChannelVisualizer;

//
// Visualizes communication channels between network nodes on a canvas. Displays
// arrows representing active communication channels with customizable appearance
// (color, style, width). Shows packet information as labels on the arrows and
// supports animation effects like fading for inactive channels. Provides visual
// feedback about network traffic patterns and helps with debugging communication
// between nodes in the simulation.
//
simple ChannelCanvasVisualizer extends ChannelVisualizerBase like IChannelVisualizer
{
    parameters:
        double zIndex = default(1); // Determines the drawing order of figures relative to other visualizers.
        @class(ChannelCanvasVisualizer);
}

