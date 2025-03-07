//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for scene visualizers. Scene visualizers display the simulation
// environment, coordinate axes, and set up the initial viewpoint for the
// visualization.
//
// @see ~SceneCanvasVisualizer, ~SceneOsgVisualizer, ~SceneVisualizer, ~SceneVisualizerBase
//
moduleinterface ISceneVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}
