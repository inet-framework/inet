//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.scene;

import inet.visualizer.contract.ISceneVisualizer;
import inet.visualizer.osg.base.SceneOsgVisualizerBase;

//
// This module visualizes the scene using osg. It displays the scene and
// the coordinate system axes. It sets the initial viewpoint of the 3D scene
// based on the bounding sphere of all network nodes. The viewpoint is located
// opposite of the coordinate system origin with respect to the sphere, and it
// faces towards the center of the sphere.
//
// @see ~SceneCanvasVisualizer, ~SceneVisualizer, ~SceneVisualizerBase, ~ISceneVisualizer
//
simple SceneOsgVisualizer extends SceneOsgVisualizerBase like ISceneVisualizer
{
    parameters:
        @class(SceneOsgVisualizer);
        clearColor = default("#FFFFFF"); // scene background color, white by default
}

