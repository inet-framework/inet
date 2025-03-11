//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.linklayer;

import inet.visualizer.base.Ieee80211VisualizerBase;
import inet.visualizer.contract.IIeee80211Visualizer;

//
// Visualizes IEEE 802.11 wireless LAN associations in a 3D scene using OpenSceneGraph (OSG).
// Displays connection status between wireless stations and access points.
//
// The visualization includes:
// - Icons showing wireless interfaces with their associated network names (SSIDs)
// - Different icons for access points and stations TODO ? also, does it have the variable power icons?
// - Automatic updates when association status changes
// - Filtering options to visualize only specific networks or nodes
//
// Particularly useful for understanding wireless connectivity patterns,
// analyzing Wi-Fi network coverage, debugging association issues, and
// creating visual representations of wireless network topologies in 3D view.
//
// @see ~Ieee80211CanvasVisualizer, ~Ieee80211Visualizer, ~Ieee80211VisualizerBase, ~IIeee80211Visualizer
//
simple Ieee80211OsgVisualizer extends Ieee80211VisualizerBase like IIeee80211Visualizer
{
    parameters:
        @class(Ieee80211OsgVisualizer);
}
