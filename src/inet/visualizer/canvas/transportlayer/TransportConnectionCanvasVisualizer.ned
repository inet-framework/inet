//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.transportlayer;

import inet.visualizer.base.TransportConnectionVisualizerBase;
import inet.visualizer.contract.ITransportConnectionVisualizer;

//
// This module visualizes transport layer connections on a 2D canvas.
// It displays a pair of colored icons for each connection at the network nodes
// where the connection starts or ends.
//
// @see ~TransportConnectionOsgVisualizer, ~TransportConnectionVisualizer, ~TransportConnectionVisualizerBase, ~ITransportConnectionVisualizer
//
simple TransportConnectionCanvasVisualizer extends TransportConnectionVisualizerBase like ITransportConnectionVisualizer
{
    parameters:
        double zIndex = default(4); // determines the drawing order of figures relative to other visualizers
        @class(TransportConnectionCanvasVisualizer);
}

