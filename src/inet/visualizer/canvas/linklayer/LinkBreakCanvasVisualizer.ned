//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.linklayer;

import inet.visualizer.base.LinkBreakVisualizerBase;
import inet.visualizer.contract.ILinkBreakVisualizer;

//
// This module visualizes link breaks on a 2D canvas. It displays an icon
// for each link break in the network. See the documentation of the base
// module for what constitutes a link break. The icon is positioned at the
// middle point of the link.
//
// @see ~LinkBreakOsgVisualizer, ~LinkBreakVisualizer, ~LinkBreakVisualizerBase, ~ILinkBreakVisualizer
//
simple LinkBreakCanvasVisualizer extends LinkBreakVisualizerBase like ILinkBreakVisualizer
{
    parameters:
        double zIndex = default(2); // determines the drawing order of figures relative to other visualizers
        @class(LinkBreakCanvasVisualizer);
}

