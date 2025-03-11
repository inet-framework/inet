//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.linklayer;

import inet.visualizer.base.DataLinkVisualizerBase;
import inet.visualizer.contract.IDataLinkVisualizer;

//
// Visualizes data link layer connections in a 3D scene using OpenSceneGraph (OSG).
// Displays directional arrows representing active link layer communication between
// network nodes.
//
// The visualization includes:
// - 3D arrows showing active data link layer connections
// - Directional indicators showing packet flow from source to destination
// - Separate arrows for bidirectional communication
// - Temporal fading as links become inactive over time
// - Automatic updates when new packets travel along links
// - Dynamic positioning that follows mobile node movement
//
// A data link becomes active when a packet is successfully sent from one network
// interface to another at the link layer. The arrow representing this link
// gradually fades unless reinforced by subsequent packets following the same path.
//
// Particularly useful for understanding MAC-level connectivity patterns,
// analyzing link layer protocol behavior, debugging link establishment issues,
// and creating dynamic visualizations of link layer activity in 3D view.
//
// @see ~DataLinkCanvasVisualizer, ~DataLinkVisualizer, ~DataLinkVisualizerBase, ~IDataLinkVisualizer
//
simple DataLinkOsgVisualizer extends DataLinkVisualizerBase like IDataLinkVisualizer
{
    parameters:
        @class(DataLinkOsgVisualizer);
}
