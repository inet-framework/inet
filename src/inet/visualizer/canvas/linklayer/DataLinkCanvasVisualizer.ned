//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.linklayer;

import inet.visualizer.base.DataLinkVisualizerBase;
import inet.visualizer.contract.IDataLinkVisualizer;

//
// Visualizes data link layer activity on a 2D canvas. Displays arrows representing
// active links between network nodes at the data link layer.
//
// The visualization includes:
// - Arrows pointing from source to destination nodes
// - Packet names displayed on the arrows
// - Gradual fading of arrows as links become inactive
// - Dynamic updates when nodes move
// - Multiple observation levels (service, peer, protocol) for different aspects of link activity
// - Filtering options to visualize only specific packet types, interfaces, or nodes
//
// Particularly useful for understanding network traffic patterns, analyzing protocol
// behavior in wireless networks, and troubleshooting connectivity issues.
//
// @see ~DataLinkOsgVisualizer, ~DataLinkVisualizer, ~DataLinkVisualizerBase, ~IDataLinkVisualizer
//
simple DataLinkCanvasVisualizer extends DataLinkVisualizerBase like IDataLinkVisualizer
{
    parameters:
        double zIndex = default(2); // Determines the drawing order of figures relative to other visualizers
        @class(DataLinkCanvasVisualizer);
}
