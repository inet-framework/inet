//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.scene;

import inet.visualizer.contract.ISceneVisualizer;
import inet.visualizer.vsg.base.SceneVsgVisualizerBase;

//
// Visualizes the scene using VulkanSceneGraph. It displays the scene and the
// coordinate system axes, and sets the initial viewpoint of the 3D scene based
// on the bounding sphere of all network nodes.
//
// @see ~SceneCanvasVisualizer, ~SceneVisualizer, ~SceneVisualizerBase, ~ISceneVisualizer
//
simple SceneVsgVisualizer extends SceneVsgVisualizerBase like ISceneVisualizer
{
    parameters:
        @class(SceneVsgVisualizer);
        clearColor = default("#FFFFFF"); // Scene background color, default is white
}
