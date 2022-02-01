//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.common;

import inet.visualizer.base.GateVisualizerBase;
import inet.visualizer.contract.IGateVisualizer;

//
// This module visualizes gates on a 2D canvas.
//
// @see ~GateOsgVisualizer, ~GateVisualizer, ~GateVisualizerBase, ~IGateVisualizer
//
simple GateCanvasVisualizer extends GateVisualizerBase like IGateVisualizer
{
    parameters:
        double zIndex = default(10); // determines the drawing order of figures relative to other visualizers
        @class(GateCanvasVisualizer);
}

