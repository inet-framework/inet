//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.mobility;

import inet.visualizer.base.MobilityVisualizerBase;
import inet.visualizer.contract.IMobilityVisualizer;

//
// This module visualizes multiple mobilities on a 2D canvas. It moves visual
// representations according to their mobility model. It also displays recent
// movement as a polyline, current velocity vector and current orientation.
//
// @see ~MobilityOsgVisualizer, ~MobilityVisualizer, ~MobilityVisualizerBase, ~MobilityVisualizer
//
simple MobilityCanvasVisualizer extends MobilityVisualizerBase like IMobilityVisualizer
{
    parameters:
        double zIndex = default(0); // determines the drawing order of figures relative to other visualizers
        @class(MobilityCanvasVisualizer);
}

