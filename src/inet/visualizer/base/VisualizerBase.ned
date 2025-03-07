//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// Serves as a base module for all visualizer simple modules. Visualizers add graphical
// annotations to network elements or draw lines/shapes on the canvas to represent
// various aspects of the simulation. This base module provides common parameters and
// functionality for determining what to visualize and where to display the visualization.
//
// Visualizers typically operate by subscribing to signals emitted by simulation modules,
// processing the received information, and creating/updating visual representations
// accordingly. The visual elements are added to either a canvas (2D) or OSG (3D) scene.
//
simple VisualizerBase
{
    parameters:
        string visualizationTargetModule = default("^"); // Module path relative to the visualizer which determines where the visualization should appear in the user interface, parent module by default
        string visualizationSubjectModule = default("^"); // Module path relative to the visualizer which determines what part of the simulation model the visualizer displays in the user interface, parent module by default
        string tags = default(""); // Tag added to each visualization for disambiguation in the runtime environment
        @class(VisualizerBase);
        @display("i=block/app");
}
