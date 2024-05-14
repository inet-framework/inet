//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.physicallayer;

import inet.visualizer.base.ChannelVisualizerBase;
import inet.visualizer.contract.IChannelVisualizer;

simple ChannelCanvasVisualizer extends ChannelVisualizerBase like IChannelVisualizer
{
    parameters:
        double zIndex = default(1); // Determines the drawing order of figures relative to other visualizers.
        @class(ChannelCanvasVisualizer);
}

