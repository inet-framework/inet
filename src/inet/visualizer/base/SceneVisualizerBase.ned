//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for scene visualizer simple modules. It provides the
// fundamental spatial context for all other visualizers by determining the
// bounds of the simulation area. TODO!
//
// The scene visualizer calculates the minimum area that encloses all physical
// objects and the constraint areas of all mobile nodes. This establishes the
// coordinate system and spatial boundaries for the entire visualization.
//
// The visualizer can optionally display coordinate axes to help with spatial
// orientation, and it provides a timer mechanism that enables smooth animation
// and periodic user interface updates independent of the simulation events.
//
// The scene visualizer is typically the first visualizer to be initialized and
// serves as the foundation for all other visualizations. It determines how the
// simulation space is mapped to the screen or 3D view.
//
// The actual rendering of the scene is implemented in derived modules for
// specific user interfaces (canvas or OSG). The OSG version provides a more
// realistic 3D visualization with perspective and camera controls.
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
