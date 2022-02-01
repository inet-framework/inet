//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.common;

import inet.visualizer.base.InfoVisualizerBase;
import inet.visualizer.contract.IInfoVisualizer;

//
// This module visualizes some information of a submodule for each network node
// on a 2D canvas.
//
// @see ~InfoOsgVisualizer, ~InfoVisualizer, ~InfoVisualizerBase, ~IInfoVisualizer
//
simple InfoCanvasVisualizer extends InfoVisualizerBase like IInfoVisualizer
{
    parameters:
        double zIndex = default(10); // determines the drawing order of figures relative to other visualizers
        @class(InfoCanvasVisualizer);
}

