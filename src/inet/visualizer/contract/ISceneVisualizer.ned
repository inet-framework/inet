//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// This module interface allows configuring different ~ISceneVisualizer
// submodules from INI files. Such a visualizer displays the scene, the
// coordinate axes, sets up the initial viewpoint, etc.
//
// @see ~SceneCanvasVisualizer, ~SceneOsgVisualizer, ~SceneVisualizer, ~SceneVisualizerBase
//
moduleinterface ISceneVisualizer
{
    parameters:
        @display("i=block/app");
}

