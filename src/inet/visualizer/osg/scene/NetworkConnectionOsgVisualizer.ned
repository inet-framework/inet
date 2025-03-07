//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.scene;

import inet.visualizer.base.NetworkConnectionVisualizerBase;
import inet.visualizer.contract.INetworkConnectionVisualizer;

//
// Visualizes network connections on a 3D scene using OpenSceneGraph (OSG).
// Displays lines representing physical or logical connections between network nodes.
//
// The visualization includes:
// - 3D lines showing connections between network devices
// - Support for filtering which connections are displayed
// - Integration with the standard OMNeT++ connection visualization in 3D
//
// Note: This visualizer is typically disabled by default because OMNeT++
// already displays network connections in its default visualization.
//
// Particularly useful when custom 3D connection visualization is needed beyond
// what OMNeT++ provides by default, especially for complex network topologies
// or when connections need to be highlighted in 3D space.
//
// @see ~NetworkConnectionCanvasVisualizer, ~NetworkConnectionVisualizer, ~NetworkConnectionVisualizerBase, ~INetworkConnectionVisualizer
//
simple NetworkConnectionOsgVisualizer extends NetworkConnectionVisualizerBase like INetworkConnectionVisualizer
{
    parameters:
        @class(NetworkConnectionOsgVisualizer);
}
