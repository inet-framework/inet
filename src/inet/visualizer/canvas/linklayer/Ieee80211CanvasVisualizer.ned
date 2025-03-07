//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.linklayer;

import inet.visualizer.base.Ieee80211VisualizerBase;
import inet.visualizer.contract.IIeee80211Visualizer;

//
// Visualizes IEEE 802.11 associations on a 2D canvas. Displays information
// about wireless network connections between access points and stations.
//
// The visualization includes:
// - Icons labeled with SSIDs for each associated wireless interface
// - Visual indication of which stations are connected to which access points
// - Automatic updates when associations are established or broken
// - Filtering options to visualize only specific interfaces or nodes
//
// Particularly useful for understanding wireless network topology, debugging
// association issues, monitoring handovers between access points, and
// analyzing wireless network coverage.
//
// @see ~Ieee80211OsgVisualizer, ~Ieee80211Visualizer, ~Ieee80211VisualizerBase, ~IIeee80211Visualizer
//
simple Ieee80211CanvasVisualizer extends Ieee80211VisualizerBase like IIeee80211Visualizer
{
    parameters:
        double zIndex = default(2); // Determines the drawing order of figures relative to other visualizers
        @class(Ieee80211CanvasVisualizer);
}
