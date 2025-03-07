//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.linklayer;

import inet.visualizer.base.LinkBreakVisualizerBase;
import inet.visualizer.contract.ILinkBreakVisualizer;

//
// Visualizes link breaks on a 2D canvas. Displays icons at locations where
// communication links between network nodes fail.
//
// The visualization includes:
// - Icons appearing at the midpoint of broken links
// - Visual indication of when and where link failures occur
// - Automatic updates when new link breaks are detected
// - Filtering options to visualize only specific link types or nodes
//
// Particularly useful for understanding network topology changes, debugging
// mobility-related connectivity issues, analyzing wireless link reliability,
// and identifying problematic areas in the network.
//
// @see ~LinkBreakOsgVisualizer, ~LinkBreakVisualizer, ~LinkBreakVisualizerBase, ~ILinkBreakVisualizer
//
simple LinkBreakCanvasVisualizer extends LinkBreakVisualizerBase like ILinkBreakVisualizer
{
    parameters:
        double zIndex = default(2); // Determines the drawing order of figures relative to other visualizers
        @class(LinkBreakCanvasVisualizer);
}
