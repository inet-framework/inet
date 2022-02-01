//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.scene;

import inet.visualizer.base.NetworkNodeVisualizerBase;
import inet.visualizer.contract.INetworkNodeVisualizer;

//
// This module visualizes multiple network nodes on a 2D canvas. It maintains a
// NetworkNodeCanvasVisualization figure for each node in the network which can
// be annotated by other visualizers.
//
// @see ~NetworkNodeCanvasVisualizer, ~NetworkNodeVisualizer, ~NetworkNodeVisualizerBase, ~INetworkNodeVisualizer
//
simple NetworkNodeCanvasVisualizer extends NetworkNodeVisualizerBase like INetworkNodeVisualizer
{
    parameters:
        double zIndex = default(0); // determines the drawing order of figures relative to other visualizers
        @class(NetworkNodeCanvasVisualizer);
}

