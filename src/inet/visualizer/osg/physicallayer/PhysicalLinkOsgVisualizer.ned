//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.physicallayer;

import inet.visualizer.base.PhysicalLinkVisualizerBase;
import inet.visualizer.contract.IPhysicalLinkVisualizer;

//
// Visualizes physical layer links in a 3D scene using OpenSceneGraph (OSG).
// Displays directional arrows representing active wireless connections between
// network nodes.
//
// The visualization includes:
// - 3D arrows showing active physical layer links between nodes
// - Directional indicators showing transmission source and destination
// - Temporal fading as links become inactive over time
// - Automatic updates as new transmissions occur
// - Dynamic positioning that follows mobile node movement
// - Separate arrows for bidirectional communication
//
// A physical link becomes active when a packet is successfully received at the
// physical layer. The arrow representing this link gradually fades unless
// reinforced by subsequent packet receptions.
//
// Particularly useful for understanding wireless connectivity patterns,
// analyzing physical layer communication success, visualizing network topology
// at the physical layer, and creating dynamic visualizations of wireless
// network activity.
//
// @see ~PhysicalLinkCanvasVisualizer, ~PhysicalLinkVisualizer, ~PhysicalLinkVisualizerBase, ~IPhysicalLinkVisualizer
//
simple PhysicalLinkOsgVisualizer extends PhysicalLinkVisualizerBase like IPhysicalLinkVisualizer
{
    parameters:
        @class(PhysicalLinkOsgVisualizer);
}
