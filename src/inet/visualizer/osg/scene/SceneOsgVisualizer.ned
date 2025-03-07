//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.scene;

import inet.visualizer.contract.ISceneVisualizer;
import inet.visualizer.osg.base.SceneOsgVisualizerBase;

//
// Visualizes the simulation scene in 3D using OpenSceneGraph (OSG). Provides
// the coordinate system and viewpoint that serves as the foundation for all
// other OSG visualizers.
//
// The visualization includes:
// - 3D coordinate system axes
// - Configurable background color
// - Automatic initial viewpoint calculation based on network node positions
// - Full 3D perspective view with camera controls
//
// The initial viewpoint is intelligently positioned opposite to the coordinate
// system origin with respect to the bounding sphere of all network nodes, facing
// toward the center of the network for optimal viewing.
//
// Particularly useful for creating realistic 3D visualizations, understanding
// spatial relationships in the network, and producing high-quality visual
// presentations of simulations.
//
// @see ~SceneCanvasVisualizer, ~SceneVisualizer, ~SceneVisualizerBase, ~ISceneVisualizer
//
simple SceneOsgVisualizer extends SceneOsgVisualizerBase like ISceneVisualizer
{
    parameters:
        @class(SceneOsgVisualizer);
        clearColor = default("#FFFFFF"); // Scene background color, default is white
}
