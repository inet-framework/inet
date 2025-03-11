//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.mobility;

import inet.visualizer.base.MobilityVisualizerBase;
import inet.visualizer.contract.IMobilityVisualizer;

//
// Visualizes node mobility in a 3D scene using OpenSceneGraph (OSG). Displays
// movement patterns, velocity, and orientation of mobile network nodes.
//
// The visualization includes:
// - Dynamic positioning of nodes according to their mobility models
// - Movement trails showing recent node trajectories as 3D polylines
// - Velocity vectors indicating current speed and direction
// - Orientation indicators showing which way nodes are facing
// - Automatic updates as nodes move through the 3D environment
//
// Particularly useful for understanding movement patterns in mobile networks,
// analyzing spatial relationships between nodes, debugging mobility models,
// and creating realistic visualizations of node movement in 3D view.
//
// @see ~MobilityCanvasVisualizer, ~MobilityVisualizer, ~MobilityVisualizerBase, ~IMobilityVisualizer
//
simple MobilityOsgVisualizer extends MobilityVisualizerBase like IMobilityVisualizer
{
    parameters:
        @class(MobilityOsgVisualizer);
}
