//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.linklayer;

import inet.visualizer.base.LinkBreakVisualizerBase;
import inet.visualizer.contract.ILinkBreakVisualizer;

//
// Visualizes link failures in a 3D scene using OpenSceneGraph (OSG). Displays
// indicators at locations where communication links between network nodes break.
//
// The visualization includes:
// - 3D icons showing where link breaks occur
// - Positioning at the midpoint between the affected nodes
// - Automatic updates when new link failures happen
// - Filtering options to visualize only specific link types or nodes
//
// A link break occurs when a previously working communication link between
// two network nodes fails. This typically happens due to node mobility (moving
// out of range), interference, or explicit link failures in wired networks.
//
// Particularly useful for understanding connectivity problems in mobile networks,
// analyzing network resilience, debugging routing protocol reactions to topology
// changes, and creating visual representations of network dynamics in 3D space.
//
// @see ~LinkBreakCanvasVisualizer, ~LinkBreakVisualizer, ~LinkBreakVisualizerBase, ~ILinkBreakVisualizer
//
simple LinkBreakOsgVisualizer extends LinkBreakVisualizerBase like ILinkBreakVisualizer
{
    parameters:
        @class(LinkBreakOsgVisualizer);
}
