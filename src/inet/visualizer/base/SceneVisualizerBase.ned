//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for scene visualizer simple modules. It determines the
// bounds of the scene, that is the minimum area where the whole simulation
// fits in. The scene encloses all physical objects and the constraint area
// of all mobile nodes. The visualizer also provides a timer which can be used
// to periodically update the user interface independently of the underlying
// simulation. The actual visualization of the scene is done in derived modules.
//
// @see ~SceneCanvasVisualizer, ~SceneOsgVisualizer, ~ISceneVisualizer, ~VisualizerBase
//
simple SceneVisualizerBase extends VisualizerBase
{
    parameters:
        string physicalEnvironmentModule = default("physicalEnvironment"); // the physical environment module, refers to a top level submodule by default
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // the network node visualizer, refers to a sibling submodule by default
        double axisLength @mutable @unit(m) = default(nan m); // length of x, y, and z axes, no axis by default
        double animationSpeed = default(0); // always active animation speed independent of any visualization, value must be in the range (0, +inf)
        double sceneMinX @unit(m) = default(nan m); // lower bound of x coordinates in scene
        double sceneMinY @unit(m) = default(nan m); // lower bound of y coordinates in scene
        double sceneMinZ @unit(m) = default(nan m); // lower bound of z coordinates in scene
        double sceneMaxX @unit(m) = default(nan m); // upper bound of x coordinates in scene
        double sceneMaxY @unit(m) = default(nan m); // upper bound of y coordinates in scene
        double sceneMaxZ @unit(m) = default(nan m); // upper bound of z coordinates in scene
        @class(SceneVisualizerBase);
}
