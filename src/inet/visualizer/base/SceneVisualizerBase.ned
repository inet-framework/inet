//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for scene visualizer simple modules. It determines the
// bounds of the scene, which is the minimum area where the whole simulation
// fits in. The scene encloses all physical objects and the constraint area
// of all mobile nodes. The visualizer also provides a timer that can be used
// to periodically update the user interface independently of the underlying
// simulation. The actual visualization of the scene is done in derived modules.
//
// @see ~SceneCanvasVisualizer, ~SceneOsgVisualizer, ~ISceneVisualizer, ~VisualizerBase
//
simple SceneVisualizerBase extends VisualizerBase
{
    parameters:
        string physicalEnvironmentModule = default("physicalEnvironment"); // The physical environment module, refers to a top-level submodule by default
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // The network node visualizer, refers to a sibling submodule by default
        double axisLength @mutable @unit(m) = default(nan m); // Length of x, y, and z axes, no axis by default
        double animationSpeed = default(0); // Always active animation speed independent of any visualization, the value must be in the range (0, +inf)
        double sceneMinX @unit(m) = default(nan m); // Lower bound of x coordinates in the scene
        double sceneMinY @unit(m) = default(nan m); // Lower bound of y coordinates in the scene
        double sceneMinZ @unit(m) = default(nan m); // Lower bound of z coordinates in the scene
        double sceneMaxX @unit(m) = default(nan m); // Upper bound of x coordinates in the scene
        double sceneMaxY @unit(m) = default(nan m); // Upper bound of y coordinates in the scene
        double sceneMaxZ @unit(m) = default(nan m); // Upper bound of z coordinates in the scene
        @class(SceneVisualizerBase);
}
