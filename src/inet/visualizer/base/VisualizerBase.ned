//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// Serves as the foundation for all visualizer simple modules in the INET framework.
// This module provides the core functionality and parameters that are common to
// all visualizers, establishing a consistent architecture for visualization components.
//
// Visualizers enhance simulation understanding by adding graphical annotations to
// network elements or drawing lines/shapes on the canvas to represent various aspects
// of the simulation. They make complex network behavior visible and help debug
// protocol implementations by providing visual feedback.
//
// The visualization architecture in INET follows a layered design:
// - VisualizerBase: Provides common parameters and core functionality
// - Specialized base modules: Extend VisualizerBase for specific visualization tasks
// - Canvas/OSG implementations: Implement the actual rendering for 2D or 3D scenes
//
// Visualizers typically operate by:
// 1. Subscribing to signals emitted by simulation modules
// 2. Processing the received information to determine what to display
// 3. Creating/updating visual representations in the user interface
// 4. Managing the lifecycle of visual elements as the simulation progresses
//
// The visualizationTargetModule parameter determines where the visualization appears, TODO
// while visualizationSubjectModule specifies what part of the simulation is visualized.
// This separation allows for flexible configuration of visualizations.
//
// @see ~IVisualizer, ~IntegratedVisualizer
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
