//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.base;

import inet.visualizer.base.SceneVisualizerBase;

//
// Base module for scene visualizer simple modules on a 3D osg scene.
// It displays the scene either as a filled rectangle or as an image.
//
// @see ~SceneOsgVisualizer, ~SceneOsgEarthVisualizer, ~SceneVisualizerBase
//
simple SceneOsgVisualizerBase extends SceneVisualizerBase
{
    parameters:
        string cameraManipulator @enum("auto","trackball","overview","terrain","earth") = default("auto"); // Type of camera manipulator
        double cameraDistanceFactor = default(5); // Camera distance relative to the network bounding sphere radius
        double zNear @unit(m) = default(1m); // Near distance for Z buffer optimization
        double zFar @unit(m) = default(100000m); // Far distance for Z buffer optimization
        double fieldOfView @unit(deg) = default(30deg); // Camera field of view
        string clearColor = default(""); // Scene background color, not set by default

        bool displayScene = default(true); // Display the scene, enabled by default
        string sceneColor = default("#A0E0A0"); // Scene solid color if scene texture is not provided, gray by default
        double sceneOpacity = default(1.0); // Scene opacity, opaque by default (not recommended to change)
        string sceneImage = default(""); // Scene texture (repeated), no image by default
        double sceneImageSize @unit(m) = default(1m); // Scene texture size
        bool sceneShading = default(true); // Exponential shading for scene color, enabled by default

        @class(SceneOsgVisualizerBase);
}

