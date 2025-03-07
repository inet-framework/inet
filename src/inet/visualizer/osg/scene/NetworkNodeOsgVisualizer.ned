//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.scene;

import inet.visualizer.base.NetworkNodeVisualizerBase;
import inet.visualizer.contract.INetworkNodeVisualizer;

//
// Visualizes network nodes in a 3D scene using OpenSceneGraph (OSG). Creates and
// maintains visual representations of network devices that serve as the foundation
// for other 3D visualizations.
//
// The visualization includes:
// - 3D models or camera-facing 2D icons representing each network node
// - Optional display of node names for easy identification
// - Support for node filtering to display only specific nodes
// - A framework for other visualizers to attach 3D annotations to nodes
//
// Each node can be represented either by:
// - A 3D model specified in the node's 'osgModel' parameter
// - A 2D icon from the node's display string that automatically rotates to face the camera
//
// Particularly useful as the base visualization layer for 3D simulations,
// providing realistic device representations, creating immersive visualizations,
// and supporting complex 3D network topologies.
//
// @see ~NetworkNodeCanvasVisualizer, ~NetworkNodeVisualizer, ~NetworkNodeVisualizerBase, ~INetworkNodeVisualizer
//
simple NetworkNodeOsgVisualizer extends NetworkNodeVisualizerBase like INetworkNodeVisualizer
{
    parameters:
        @class(NetworkNodeOsgVisualizer);
        bool displayModuleName = default(true); // Display network node name, enabled by default
}
